/*
 * test_mandelbrot.c — тесты для mandelbrot_contains на точках,
 * про которые ответ известен заранее.
 *
 * Запуск:  make test
 */
#include <stdio.h>
#include <math.h>
#include "mandelbrot.h"

typedef struct {
   double      x, y;       /* c = x + i*y */
   bool        expected;   /* true — точка в множестве */
   const char* why;
} test_case_t;

static const test_case_t TESTS[] = {
   /* ---- точки, которые ДОЛЖНЫ быть в множестве ---- */
   {      0.0,     0.0, true,  "0 -> 0 -> 0 ... (неподвижная точка)" },
   {     -1.0,     0.0, true,  "0 -> -1 -> 0 -> -1 ... (цикл длины 2)" },
   {     -2.0,     0.0, true,  "0 -> -2 -> 2 -> 2 ... (крайняя левая точка)" },
   {      0.0,     1.0, true,  "c = i: 0 -> i -> -1+i -> -i -> -1+i ... (цикл)" },
   {      0.0,    -1.0, true,  "c = -i: симметрично c = i" },
   {     0.25,     0.0, true,  "острие кардиоиды: z_n -> 0.5" },
   {    -1.75,     0.0, true,  "весь отрезок [-2; 0.25] лежит в множестве" },
   {     -0.5,     0.5, true,  "внутри главной кардиоиды" },
   {     -0.1,     0.6, true,  "внутри главной кардиоиды, у верхнего края" },
   {  -0.1226,  0.7449, true,  "центр верхней почки (цикл длины 3)" },
   {     -1.0,     0.2, true,  "внутри круга |c + 1| < 1/4" },

   /* ---- точки, которые НЕ должны быть в множестве ---- */
   {      1.0,     0.0, false, "0 -> 1 -> 2 -> 5 -> 26 ... (быстро уходит)" },
   {      0.3,     0.0, false, "правее 0.25 на вещественной оси" },
   {     0.26,     0.0, false, "чуть правее острия, ~30 шагов" },
   {    -2.01,     0.0, false, "левее -2" },
   {      0.0,     2.0, false, "|c| = 2, но не -2: уходит" },
   {      1.0,     1.0, false, "|c| > 2: по теореме вне множества" },
   {      0.5,     0.5, false, "справа от кардиоиды" },
   {     -0.1,    0.65, false, "рядом с кардиоидой, но снаружи: 75 шагов" },
   {    -0.75,     0.1, false, "в 'перешейке' между кардиоидой и кругом" },
};

static const char* yes_no(bool v) { return v ? "IN " : "OUT"; }

/* Точки c = 0.25 + eps вне множества, но уходят лишь за ~pi/sqrt(eps)
   шагов: показывает, что конечного MAX_ITER не хватает у границы */
static void demo_max_iter_limit(void) {
   const double eps[] = { 1e-2, 1e-4, 1e-6, 1e-8 };

   printf("\nТочки c = 0.25 + eps (все ВНЕ множества):\n");
   for (int i = 0; i < 4; i++) {
      bool got = mandelbrot_contains(0.25 + eps[i]);   /* мнимая часть 0 */
      printf("  eps = %.0e  нужно ~%7.0f итераций -> %s%s\n",
             eps[i], 3.14159265358979 / sqrt(eps[i]), yes_no(got),
             got ? "  (ложное IN: не хватило MAX_ITER, это ожидаемо)" : "");
   }
}

int main(void) {
   int total  = sizeof(TESTS) / sizeof(TESTS[0]);
   int failed = 0;

   printf("MAX_ITER = %d\n\n", MANDELBROT_MAX_ITER);
   for (int i = 0; i < total; i++) {
      const test_case_t* t = &TESTS[i];
      bool got = mandelbrot_contains(t->x + t->y * I);
      bool ok  = (got == t->expected);
      if (!ok) failed++;

      printf("[%s] c = %7.4f %+7.4fi  ожидали %s получили %s  %s\n",
             ok ? " OK " : "FAIL", t->x, t->y,
             yes_no(t->expected), yes_no(got), t->why);
   }

   demo_max_iter_limit();

   printf("\n%s: %d из %d тестов прошли\n",
          failed ? "ЕСТЬ ОШИБКИ" : "ВСЁ ВЕРНО", total - failed, total);
   return failed ? 1 : 0;
}
