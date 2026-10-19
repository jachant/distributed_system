#ifndef WORKLOAD_H
#define WORKLOAD_H

#include "domain.h"
#include "list.h"

/* Заполняет пустой список: пытается вставить w.initial_inserts
   случайных ключей, сдаётся после 2 * initial_inserts попыток.
   Возвращает, сколько ключей вставлено. */
long workload_fill(list_t* list, workload_t w);

/* Выполняет w.total_ops операций на w.thread_count потоках.
   Возвращает 0 при успехе, -1 если не хватило памяти. */
int  workload_run(list_t* list, workload_t w, workload_result_t* result);

#endif
