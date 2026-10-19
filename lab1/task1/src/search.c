#include <stdlib.h>
#include <pthread.h>

#include "search.h"
#include "mandelbrot.h"
#include "my_rand.h"
#include "timer.h"

/* Задание для одного потока */
typedef struct {
   long            rank;    /* номер потока                                  */
   region_t        region;  /* где искать                                    */
   double complex* out;     /* начало участка массива, принадлежащего потоку */
   long            need;    /* сколько точек должен найти                    */
   long            tested;  /* сколько точек проверил (заполняет поток)      */
} task_t;

/* Случайная точка, равномерно распределённая в области */
static double complex random_point(region_t r, unsigned* seed) {
   double x = r.x_min + (r.x_max - r.x_min) * my_drand(seed);
   double y = r.y_min + (r.y_max - r.y_min) * my_drand(seed);
   return x + y * I;
}

/* Функция потока: искать точки, пока не найдено task->need штук */
static void* worker(void* arg) {
   task_t* task = arg;

   /* У каждого потока свой seed — иначе все потоки найдут одни и те же точки */
   unsigned seed = 1 + 7919u * (unsigned) task->rank;

   long found = 0;
   while (found < task->need) {
      double complex c = random_point(task->region, &seed);
      task->tested++;
      if (mandelbrot_contains(c))
         task->out[found++] = c;
   }
   return NULL;
}

/* Делим point_count между потоками; остаток раздаём первым потокам */
static void split_work(config_t cfg, region_t region,
                       double complex* points, task_t* tasks) {
   long base   = cfg.point_count / cfg.thread_count;
   long rest   = cfg.point_count % cfg.thread_count;
   long offset = 0;

   for (int i = 0; i < cfg.thread_count; i++) {
      tasks[i].rank   = i;
      tasks[i].region = region;
      tasks[i].need   = base + (i < rest ? 1 : 0);
      tasks[i].out    = points + offset;
      tasks[i].tested = 0;
      offset += tasks[i].need;
   }
}

int search_parallel(config_t cfg, region_t region, search_result_t* result) {
   double complex* points  = malloc(cfg.point_count * sizeof(double complex));
   task_t*         tasks   = malloc(cfg.thread_count * sizeof(task_t));
   pthread_t*      threads = malloc(cfg.thread_count * sizeof(pthread_t));
   if (points == NULL || tasks == NULL || threads == NULL) {
      free(points); free(tasks); free(threads);
      return -1;
   }

   split_work(cfg, region, points, tasks);

   double start, finish;
   GET_TIME(start);
   for (int i = 0; i < cfg.thread_count; i++)
      pthread_create(&threads[i], NULL, worker, &tasks[i]);
   for (int i = 0; i < cfg.thread_count; i++)
      pthread_join(threads[i], NULL);
   GET_TIME(finish);

   result->points  = points;
   result->elapsed = finish - start;
   result->tested  = 0;
   for (int i = 0; i < cfg.thread_count; i++)
      result->tested += tasks[i].tested;

   free(tasks);
   free(threads);
   return 0;
}
