/*
 * test_nbody.c — проверка моделирования на задачах с известным ответом.
 *
 * Запуск:  make test
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "physics.h"
#include "simulation.h"
#include "vec3.h"

#define SUN_MASS    1.989e30
#define EARTH_MASS  5.972e24
#define AU          1.496e11          /* расстояние Земля — Солнце, м */
#define PI          3.14159265358979

static int failed = 0, total = 0;

static void check(int ok, const char* what, double value, double limit) {
   total++;
   if (!ok) failed++;
   printf("[%s] %9.2e (допуск %.0e)  %s\n",
          ok ? " OK " : "FAIL", value, limit, what);
}

static vec3_t momentum(const body_t* b, long n) {
   vec3_t p = { 0, 0, 0 };
   for (long k = 0; k < n; k++) p = vec3_add(p, vec3_scale(b[k].vel, b[k].mass));
   return p;
}

static double energy(const body_t* b, long n) {
   double e = 0;
   for (long k = 0; k < n; k++) {
      e += 0.5 * b[k].mass * vec3_dot(b[k].vel, b[k].vel);
      for (long j = k + 1; j < n; j++) {
         vec3_t d = vec3_sub(b[j].pos, b[k].pos);
         e -= GRAVITY_G * b[k].mass * b[j].mass / sqrt(vec3_dot(d, d));
      }
   }
   return e;
}

/* Солнце и Земля на круговой орбите вокруг общего центра масс */
static system_t sun_earth(void) {
   system_t s = { 2, malloc(2 * sizeof(body_t)) };
   double v = sqrt(GRAVITY_G * (SUN_MASS + EARTH_MASS) / AU);  /* относит. скорость */
   double mu = EARTH_MASS / (SUN_MASS + EARTH_MASS);
   s.bodies[0] = (body_t){ SUN_MASS,   { -mu * AU, 0, 0 },       { 0, -mu * v, 0 } };
   s.bodies[1] = (body_t){ EARTH_MASS, { (1 - mu) * AU, 0, 0 },  { 0, (1 - mu) * v, 0 } };
   return s;
}

/* Случайное скопление тел (свой простой генератор, чтобы тест не зависел от rand) */
static system_t random_cluster(long n, unsigned seed) {
   system_t s = { n, malloc(n * sizeof(body_t)) };
   for (long k = 0; k < n; k++) {
      double r[7];
      for (int i = 0; i < 7; i++) {
         seed = seed * 1103515245u + 12345u;
         r[i] = (seed >> 8) / (double) (1u << 24);      /* [0; 1) */
      }
      s.bodies[k] = (body_t){ 1e22 * (1 + r[0]),
                              { 1e9 * (r[1] - .5), 1e9 * (r[2] - .5), 1e9 * (r[3] - .5) },
                              { 100 * (r[4] - .5), 100 * (r[5] - .5), 100 * (r[6] - .5) } };
   }
   return s;
}

static trajectory_t run(system_t* s, double t_end, double dt, int threads) {
   trajectory_t traj;
   config_t cfg = { .t_end = t_end, .dt = dt, .thread_count = threads };
   if (simulation_run(s, cfg, &traj) != 0) { fprintf(stderr, "no memory\n"); exit(2); }
   return traj;
}

/* 1. Тело без соседей движется равномерно и прямолинейно: r = r0 + v*t */
static void test_free_body(void) {
   system_t s = { 1, malloc(sizeof(body_t)) };
   s.bodies[0] = (body_t){ 1.0, { 1, 2, 3 }, { 4, -5, 6 } };
   trajectory_t tr = run(&s, 10.0, 0.1, 1);

   double* last = tr.xy + (tr.rows - 1) * 2;
   double err = fabs(last[0] - 41) + fabs(last[1] - (-48));
   check(err < 1e-9, "одно тело: x = x0 + vx*t, y = y0 + vy*t", err, 1e-9);
   trajectory_free(&tr); free(s.bodies);
}

/* Земля после одного оборота: насколько она промахнулась мимо старта
   (в долях AU) и как сильно радиус орбиты отклонялся от 1 AU */
static void earth_orbit_errors(double dt, double* miss, double* radius_err) {
   system_t s = sun_earth();
   double period = 2 * PI * sqrt(AU * AU * AU / (GRAVITY_G * (SUN_MASS + EARTH_MASS)));
   double x0 = s.bodies[1].pos.x, y0 = s.bodies[1].pos.y;

   trajectory_t tr = run(&s, period, dt, 2);
   double* earth = tr.xy + ((tr.rows - 1) * 2 + 1) * 2;
   *miss = hypot(earth[0] - x0, earth[1] - y0) / AU;

   *radius_err = 0;
   for (long row = 0; row < tr.rows; row++) {
      double* p = tr.xy + row * 2 * 2;
      double r = hypot(p[2] - p[0], p[3] - p[1]);
      *radius_err = fmax(*radius_err, fabs(r - AU) / AU);
   }
   trajectory_free(&tr); free(s.bodies);
}

/* 2. Земля за один период возвращается в исходную точку.
   Явный метод Эйлера немного «раскручивает» орбиту, поэтому шаг мелкий. */
static void test_orbit_period(void) {
   double miss, radius_err;
   earth_orbit_errors(60.0, &miss, &radius_err);       /* шаг — 1 минута */
   check(miss < 1e-3,       "Земля через год вернулась на место (ошибка / AU)", miss, 1e-3);
   check(radius_err < 1e-3, "орбита круговая: max |r - AU| / AU", radius_err, 1e-3);

   /* Метод Эйлера первого порядка: шаг вдвое меньше — ошибка вдвое меньше */
   double miss1, r1, miss2, r2;
   earth_orbit_errors(1200.0, &miss1, &r1);
   earth_orbit_errors(600.0,  &miss2, &r2);
   double ratio = r1 / r2;
   check(fabs(ratio - 2) < 0.1, "1-й порядок: ошибка(dt) / ошибка(dt/2) = 2", ratio, 0.1);
}

/* 3. Законы сохранения на случайном скоплении */
static void test_conservation(void) {
   system_t s = random_cluster(50, 7);
   vec3_t p0 = momentum(s.bodies, s.count);
   double e0 = energy(s.bodies, s.count);
   double scale = 0;
   for (long k = 0; k < s.count; k++)
      scale += s.bodies[k].mass * sqrt(vec3_dot(s.bodies[k].vel, s.bodies[k].vel));

   trajectory_t tr = run(&s, 1e5, 10.0, 4);
   vec3_t dp = vec3_sub(momentum(s.bodies, s.count), p0);
   double perr = sqrt(vec3_dot(dp, dp)) / scale;
   double eerr = fabs((energy(s.bodies, s.count) - e0) / e0);

   check(perr < 1e-10, "импульс системы сохраняется (отн. ошибка)", perr, 1e-10);
   check(eerr < 1e-3,  "энергия системы сохраняется (отн. ошибка)", eerr, 1e-3);
   trajectory_free(&tr); free(s.bodies);
}

/* 4. Результат не зависит от числа потоков.
   Побитового совпадения нет: при разном числе потоков силы складываются
   в разном порядке, а сложение double не ассоциативно. Разница должна
   быть на уровне ошибок округления. */
static void test_threads_identical(void) {
   const int threads[] = { 1, 2, 3, 4, 7, 16, 100 };
   system_t ref_sys = random_cluster(37, 11);
   trajectory_t ref = run(&ref_sys, 1e4, 10.0, 1);
   size_t size = (size_t) ref.rows * ref.body_count * 2;

   for (int i = 1; i < 7; i++) {
      system_t s = random_cluster(37, 11);
      trajectory_t tr = run(&s, 1e4, 10.0, threads[i]);
      double diff = 0;
      for (size_t c = 0; c < size; c++) diff = fmax(diff, fabs(tr.xy[c] - ref.xy[c]));
      diff /= 1e9;                                  /* размер скопления */

      char what[80];
      snprintf(what, sizeof what, "%3d потоков = 1 поток (max разница / размер)", threads[i]);
      check(diff < 1e-12, what, diff, 1e-12);
      trajectory_free(&tr); free(s.bodies);
   }
   trajectory_free(&ref); free(ref_sys.bodies);
}

/* 5. Один шаг совпадает с формулами (3) и (8) из методички, посчитанными
   «в лоб»: каждая сила F_qk отдельно, без третьего закона Ньютона */
static void test_one_euler_step(void) {
   system_t s = random_cluster(20, 3);
   body_t before[20];
   for (long q = 0; q < 20; q++) before[q] = s.bodies[q];

   const double dt = 1000.0;
   trajectory_t tr = run(&s, dt, dt, 3);              /* ровно один шаг */

   double pos_err = 0, vel_err = 0;
   for (long q = 0; q < 20; q++) {
      const body_t* b = &before[q];
      double fx = 0, fy = 0;                         /* формула (3) */
      for (long k = 0; k < 20; k++) {
         if (k == q) continue;
         double dx = before[k].pos.x - b->pos.x;
         double dy = before[k].pos.y - b->pos.y;
         double dz = before[k].pos.z - b->pos.z;
         double r3 = pow(dx * dx + dy * dy + dz * dz, 1.5);
         fx += GRAVITY_G * b->mass * before[k].mass * dx / r3;
         fy += GRAVITY_G * b->mass * before[k].mass * dy / r3;
      }
      double x  = b->pos.x + b->vel.x * dt;          /* формулы (8) */
      double y  = b->pos.y + b->vel.y * dt;
      double vx = b->vel.x + fx / b->mass * dt;
      double vy = b->vel.y + fy / b->mass * dt;

      pos_err = fmax(pos_err, hypot(s.bodies[q].pos.x - x, s.bodies[q].pos.y - y) / 1e9);
      vel_err = fmax(vel_err, hypot(s.bodies[q].vel.x - vx, s.bodies[q].vel.y - vy)
                              / hypot(vx - b->vel.x, vy - b->vel.y));
   }
   check(pos_err < 1e-15, "шаг Эйлера: x^n = x^{n-1} + v^{n-1} dt (старая скорость)",
         pos_err, 1e-15);
   check(vel_err < 1e-12, "шаг Эйлера: v^n = v^{n-1} + F^{n-1}/m dt (отн. ошибка dv)",
         vel_err, 1e-12);
   trajectory_free(&tr); free(s.bodies);
}

/* 6. Последняя строка — ровно tend, даже если tend не делится на dt */
static void test_time_grid(void) {
   system_t s = random_cluster(3, 1);
   trajectory_t tr = run(&s, 10.0, 3.0, 1);
   double err = fabs(tr.t[tr.rows - 1] - 10.0);
   check(err < 1e-12 && tr.rows == 5, "tend = 10, dt = 3: 4 шага по 2.5, конец ровно в tend",
         err, 1e-12);
   trajectory_free(&tr); free(s.bodies);
}

int main(void) {
   test_free_body();
   test_orbit_period();
   test_conservation();
   test_threads_identical();
   test_one_euler_step();
   test_time_grid();

   printf("\n%s: %d из %d тестов прошли\n",
          failed ? "ЕСТЬ ОШИБКИ" : "ВСЁ ВЕРНО", total - failed, total);
   return failed ? 1 : 0;
}
