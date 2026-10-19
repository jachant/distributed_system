#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <stdbool.h>
#include "domain.h"

/* Сколько итераций делаем, прежде чем считать точку принадлежащей
   множеству. Ошибка возможна только у самой границы множества. */
#define MANDELBROT_MAX_ITER 1000

/* Прямоугольник, гарантированно содержащий всё множество.
   Всё множество лежит в круге |c| <= 2; на самом деле оно уже:
   x от -2 до ~0.47, |y| до ~1.12. Берём границы с небольшим запасом. */
#define MANDELBROT_REGION ((region_t){ \
   .x_min = -2.0,  .x_max = 0.5,         \
   .y_min = -1.25, .y_max = 1.25,        \
})

/* true, если точка c принадлежит множеству Мандельброта */
bool mandelbrot_contains(double complex c);

#endif
