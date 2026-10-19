#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include "domain.h"

/* Пишет файл с заголовком "x,y" и по строке на точку.
   Возвращает 0 при успехе, -1 если файл не удалось записать. */
int csv_write_points(const char* path, const double complex* points, long count);

#endif
