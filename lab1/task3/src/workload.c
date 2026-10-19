#include <stdlib.h>
#include <pthread.h>

#include "workload.h"
#include "my_rand.h"
#include "timer.h"

/* Данные одного потока */
typedef struct {
   list_t*           list;
   void*             lock;
   workload_t        w;
   long              rank;
   long              ops;        /* сколько операций выполнить */
   workload_result_t counts;     /* что получилось (заполняет поток) */
} task_t;

long workload_fill(list_t* list, workload_t w) {
   unsigned seed = 1;
   long inserted = 0, attempts = 0;
   while (inserted < w.initial_inserts && attempts < 2 * w.initial_inserts) {
      if (list_insert(list, my_rand(&seed) % MAX_KEY)) inserted++;
      attempts++;
   }
   return inserted;
}

static void* worker(void* arg) {
   task_t*             t   = arg;
   const rwlock_api_t* api = t->w.lock;
   unsigned            seed = t->rank + 1;              /* как в pth_ll_rwl.c */
   workload_result_t   c    = { 0 };

   for (long i = 0; i < t->ops; i++) {
      double which = my_drand(&seed);
      int    value = my_rand(&seed) % MAX_KEY;

      if (which < t->w.search_percent) {
         api->rdlock(t->lock);
         list_member(t->list, value);
         api->unlock(t->lock);
         c.member_ops++;
      } else if (which < t->w.search_percent + t->w.insert_percent) {
         api->wrlock(t->lock);
         c.inserted += list_insert(t->list, value);
         api->unlock(t->lock);
         c.insert_ops++;
      } else {
         api->wrlock(t->lock);
         c.deleted += list_delete(t->list, value);
         api->unlock(t->lock);
         c.delete_ops++;
      }
   }
   t->counts = c;
   return NULL;
}

int workload_run(list_t* list, workload_t w, workload_result_t* result) {
   void*      lock    = w.lock->create();
   task_t*    tasks   = malloc(w.thread_count * sizeof(task_t));
   pthread_t* threads = malloc(w.thread_count * sizeof(pthread_t));
   if (lock == NULL || tasks == NULL || threads == NULL) {
      if (lock != NULL) w.lock->destroy(lock);
      free(tasks); free(threads);
      return -1;
   }

   /* Операции делим поровну; остаток раздаём первым потокам */
   for (int i = 0; i < w.thread_count; i++) {
      tasks[i] = (task_t){ .list = list, .lock = lock, .w = w, .rank = i,
                           .ops = w.total_ops / w.thread_count
                                + (i < w.total_ops % w.thread_count ? 1 : 0) };
   }

   double start, finish;
   GET_TIME(start);
   for (int i = 0; i < w.thread_count; i++)
      pthread_create(&threads[i], NULL, worker, &tasks[i]);
   for (int i = 0; i < w.thread_count; i++)
      pthread_join(threads[i], NULL);
   GET_TIME(finish);

   *result = (workload_result_t){ .elapsed = finish - start };
   for (int i = 0; i < w.thread_count; i++) {
      result->member_ops += tasks[i].counts.member_ops;
      result->insert_ops += tasks[i].counts.insert_ops;
      result->delete_ops += tasks[i].counts.delete_ops;
      result->inserted   += tasks[i].counts.inserted;
      result->deleted    += tasks[i].counts.deleted;
   }

   w.lock->destroy(lock);
   free(tasks);
   free(threads);
   return 0;
}
