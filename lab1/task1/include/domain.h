#ifndef DOMAIN_H
#define DOMAIN_H

#include <complex.h>

/* Прямоугольная область [x_min; x_max] x [y_min; y_max] */
typedef struct {
   double x_min, x_max;
   double y_min, y_max;
} region_t;

/* Параметры запуска из командной строки */
typedef struct {
   int  thread_count;   /* nthreads — количество потоков   */
   long point_count;    /* npoints  — сколько точек найти   */
} config_t;

/* Результат поиска */
typedef struct {
   double complex* points;  /* найденные точки, config.point_count штук */
   long            tested;  /* сколько случайных точек было проверено   */
   double          elapsed; /* время работы потоков, секунды            */
} search_result_t;

#endif
