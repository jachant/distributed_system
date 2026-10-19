#ifndef DOMAIN_H
#define DOMAIN_H

#include "rwlock_api.h"

/* Случайные ключи меньше MAX_KEY (как в pth_ll_rwl.c) */
#define MAX_KEY 100000000

/* Параметры нагрузки на список */
typedef struct {
   int                 thread_count;
   const rwlock_api_t* lock;            /* какой блокировкой защищать список */
   long                initial_inserts; /* сколько ключей вставить до старта */
   long                total_ops;       /* сколько операций всего            */
   double              search_percent;  /* доля поисков (0..1)               */
   double              insert_percent;  /* доля вставок (0..1); остальное — удаления */
} workload_t;

/* Сколько операций каждого вида выполнено */
typedef struct {
   long   member_ops, insert_ops, delete_ops;
   long   inserted, deleted;    /* из них успешных вставок и удалений */
   double elapsed;              /* время работы потоков, секунды      */
} workload_result_t;

#endif
