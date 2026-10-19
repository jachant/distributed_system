#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#include "simulation.h"
#include "physics.h"
#include "vec3.h"
#include "barrier.h"
#include "timer.h"

/* Данные, общие для всех потоков */
typedef struct {
   body_t*   bodies;
   long      count;
   int       thread_count;
   vec3_t*   local;     /* thread_count массивов сил по count штук        */
   long      steps;
   double    dt;
   double*   xy;        /* траектории, см. trajectory_t                   */
   barrier_t barrier;
} shared_t;

/* Задание для одного потока */
typedef struct {
   shared_t* shared;
   int       rank;
   long      pair_first;   /* пары с номерами [pair_first; pair_last) — для фазы 1 */
   long      pair_last;
   long      first;        /* тела [first; last) — для фазы 2 */
   long      last;
} task_t;

/* Записать x, y тела k в строку row траектории */
static void record(double* xy, long count, long row, long k, const body_t* b) {
   double* cell = xy + (row * count + k) * 2;
   cell[0] = b->pos.x;
   cell[1] = b->pos.y;
}

/*
 * Все пары (q, k), q < k, пронумерованы подряд:
 *
 *    номер:  0      1      2      ...  n-2      n-1    ...
 *    пара:  (0,1)  (0,2)  (0,3)  ... (0,n-1)  (1,2)   ...
 *
 * По номеру пары p находит тела q и k: перешагиваем целые строки
 * (в строке q ровно n - q - 1 пар), пока p не окажется внутри строки.
 * p должен быть меньше числа пар n * (n - 1) / 2.
 */
static void pair_by_index(long n, long p, long* q, long* k) {
   long row = 0;
   while (p >= n - row - 1) {    /* пара p не в строке row */
      p -= n - row - 1;          /* перешагнуть строку row */
      row++;
   }
   *q = row;
   *k = row + 1 + p;
}

/* Фаза 1: вклады в силы от своих пар — с номерами [pair_first; pair_last) */
static void compute_pair_forces(shared_t* s, const task_t* task) {
   vec3_t* my = s->local + (long) task->rank * s->count;
   const vec3_t zero = { 0.0, 0.0, 0.0 };

   for (long q = 0; q < s->count; q++)
      my[q] = zero;

   if (task->pair_first == task->pair_last) return;    /* пар не досталось */

   long q, k;
   pair_by_index(s->count, task->pair_first, &q, &k);  /* моя первая пара */

   for (long p = task->pair_first; p < task->pair_last; p++) {
      vec3_t f = physics_pair_force(&s->bodies[q], &s->bodies[k]);
      my[q] = vec3_add(my[q], f);      /* F_qk */
      my[k] = vec3_sub(my[k], f);      /* F_kq = -F_qk */

      /* следующая пара: (q, k+1), а если строка кончилась — (q+1, q+2) */
      k++;
      if (k == s->count) {
         q++;
         k = q + 1;
      }
   }
}

/* Фаза 2: сложить вклады всех потоков и сдвинуть свои тела */
static void move_own_bodies(shared_t* s, long first, long last, long step) {
   for (long q = first; q < last; q++) {
      vec3_t force = { 0.0, 0.0, 0.0 };
      for (int t = 0; t < s->thread_count; t++)
         force = vec3_add(force, s->local[(long) t * s->count + q]);

      physics_euler_step(&s->bodies[q], force, s->dt);
      record(s->xy, s->count, step, q, &s->bodies[q]);
   }
}

static void* worker(void* arg) {
   task_t*   task = arg;
   shared_t* s    = task->shared;

   for (long step = 1; step <= s->steps; step++) {
      compute_pair_forces(s, task);
      barrier_wait(&s->barrier);

      move_own_bodies(s, task->first, task->last, step);
      barrier_wait(&s->barrier);
   }
   return NULL;
}

/* Делим работу между потоками:
   - пары (фаза 1) — поровну, доли отличаются максимум на одну пару;
     если пар меньше, чем потоков, лишним потокам достаётся пустой диапазон;
   - тела (фаза 2) — на блоки, остаток раздаём первым потокам */
static void split_work(shared_t* s, task_t* tasks) {
   long pairs = s->count * (s->count - 1) / 2;
   long base  = s->count / s->thread_count;
   long rest  = s->count % s->thread_count;
   long first = 0;

   for (int i = 0; i < s->thread_count; i++) {
      long size           = base + (i < rest ? 1 : 0);
      tasks[i].shared     = s;
      tasks[i].rank       = i;
      tasks[i].pair_first = pairs * i / s->thread_count;
      tasks[i].pair_last  = pairs * (i + 1) / s->thread_count;
      tasks[i].first      = first;
      tasks[i].last       = first + size;
      first += size;
   }
}

/* Число шагов: tend делится на целое число шагов, не длиннее dt,
   чтобы последняя строка траектории была ровно в момент tend */
static long step_count(double t_end, double dt) {
   long steps = (long) ceil(t_end / dt - 1e-9);
   return steps > 0 ? steps : 1;
}

int simulation_run(system_t* sys, config_t cfg, trajectory_t* out) {
   long steps = step_count(cfg.t_end, cfg.dt);
   long rows  = steps + 1;

   shared_t s = {
      .bodies = sys->bodies,
      .count  = sys->count,
      /* Потоков больше, чем тел, не нужно: лишние сидели бы без работы */
      .thread_count = cfg.thread_count < sys->count ? cfg.thread_count
                                                    : (int) sys->count,
      .steps  = steps,
      .dt     = cfg.t_end / steps,
   };
   int threads_n = s.thread_count;

   s.local = malloc((size_t) threads_n * sys->count * sizeof(vec3_t));
   s.xy    = malloc((size_t) rows * sys->count * 2 * sizeof(double));
   double*    t       = malloc(rows * sizeof(double));
   task_t*    tasks   = malloc(threads_n * sizeof(task_t));
   pthread_t* threads = malloc(threads_n * sizeof(pthread_t));
   if (!s.local || !s.xy || !t || !tasks || !threads) {
      free(s.local); free(s.xy); free(t); free(tasks); free(threads);
      return -1;
   }

   for (long row = 0; row < rows; row++)
      t[row] = row * s.dt;
   for (long k = 0; k < sys->count; k++)
      record(s.xy, s.count, 0, k, &sys->bodies[k]);     /* t = 0 */

   barrier_init(&s.barrier, threads_n);
   split_work(&s, tasks);

   double start, finish;
   GET_TIME(start);
   for (int i = 0; i < threads_n; i++)
      pthread_create(&threads[i], NULL, worker, &tasks[i]);
   for (int i = 0; i < threads_n; i++)
      pthread_join(threads[i], NULL);
   GET_TIME(finish);

   barrier_destroy(&s.barrier);
   free(s.local);
   free(tasks);
   free(threads);

   out->rows       = rows;
   out->body_count = sys->count;
   out->t          = t;
   out->xy         = s.xy;
   out->elapsed    = finish - start;
   return 0;
}

void trajectory_free(trajectory_t* traj) {
   free(traj->t);
   free(traj->xy);
}
