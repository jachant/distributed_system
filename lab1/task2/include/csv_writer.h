#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include "domain.h"

/* Пишет файл с заголовком "t,x1,y1,...,xn,yn" и по строке на момент
   времени. Возвращает 0 при успехе, -1 если файл не удалось записать. */
int csv_write_trajectory(const char* path, const trajectory_t* traj);

#endif
