#ifndef SEARCH_H
#define SEARCH_H

#include "domain.h"

/*
 * Находит config.point_count точек множества, используя
 * config.thread_count потоков. Каждый поток бросает случайные точки
 * в область region и сохраняет подходящие в свой участок общего массива.
 *
 * Возвращает 0 при успехе. Массив result->points освобождает
 * вызывающий через free().
 */
int search_parallel(config_t config, region_t region, search_result_t* result);

#endif
