#include "barrier.h"

void barrier_init(barrier_t* b, int count) {
   pthread_mutex_init(&b->mutex, NULL);
   pthread_cond_init(&b->all_arrived, NULL);
   b->count      = count;
   b->waiting    = 0;
   b->generation = 0;
}

void barrier_wait(barrier_t* b) {
   pthread_mutex_lock(&b->mutex);

   /* Запоминаем номер прохода: барьер используется много раз подряд,
      и ждать надо именно «своего» открытия, а не следующего */
   unsigned long my_generation = b->generation;

   if (++b->waiting == b->count) {
      /* Пришёл последний поток: открываем барьер и будим остальных */
      b->waiting = 0;
      b->generation++;
      pthread_cond_broadcast(&b->all_arrived);
   } else {
      while (my_generation == b->generation)
         pthread_cond_wait(&b->all_arrived, &b->mutex);
   }

   pthread_mutex_unlock(&b->mutex);
}

void barrier_destroy(barrier_t* b) {
   pthread_cond_destroy(&b->all_arrived);
   pthread_mutex_destroy(&b->mutex);
}
