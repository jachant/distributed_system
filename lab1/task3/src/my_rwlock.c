#include "my_rwlock.h"

int my_rwlock_init(my_rwlock_t* lock) {
   pthread_mutex_init(&lock->mutex, NULL);
   pthread_cond_init(&lock->readers_cv, NULL);
   pthread_cond_init(&lock->writers_cv, NULL);
   lock->readers_active  = 0;
   lock->readers_waiting = 0;
   lock->writers_waiting = 0;
   lock->writer_active   = false;
   return 0;
}

int my_rwlock_destroy(my_rwlock_t* lock) {
   pthread_cond_destroy(&lock->writers_cv);
   pthread_cond_destroy(&lock->readers_cv);
   pthread_mutex_destroy(&lock->mutex);
   return 0;
}

int my_rwlock_rdlock(my_rwlock_t* lock) {
   pthread_mutex_lock(&lock->mutex);

   /* Ждём, пока пишет писатель или кто-то из писателей в очереди
      (приоритет писателей, см. my_rwlock.h) */
   lock->readers_waiting++;
   while (lock->writer_active || lock->writers_waiting > 0)
      pthread_cond_wait(&lock->readers_cv, &lock->mutex);
   lock->readers_waiting--;

   lock->readers_active++;
   pthread_mutex_unlock(&lock->mutex);
   return 0;
}

int my_rwlock_wrlock(my_rwlock_t* lock) {
   pthread_mutex_lock(&lock->mutex);

   /* Ждём, пока не останется ни писателя, ни читателей */
   lock->writers_waiting++;
   while (lock->writer_active || lock->readers_active > 0)
      pthread_cond_wait(&lock->writers_cv, &lock->mutex);
   lock->writers_waiting--;

   lock->writer_active = true;
   pthread_mutex_unlock(&lock->mutex);
   return 0;
}

int my_rwlock_unlock(my_rwlock_t* lock) {
   pthread_mutex_lock(&lock->mutex);

   /* Кто освобождает: писатель или один из читателей */
   if (lock->writer_active)
      lock->writer_active = false;
   else
      lock->readers_active--;

   if (lock->writers_waiting > 0) {
      /* Ждут писатели: будим одного, но только когда вышел последний
         читатель (пока кто-то читает, писателю всё равно нельзя) */
      if (lock->readers_active == 0)
         pthread_cond_signal(&lock->writers_cv);
   } else if (lock->readers_waiting > 0) {
      /* Писателей нет, ждут читатели: будим всех сразу, читать можно
         одновременно */
      pthread_cond_broadcast(&lock->readers_cv);
   }

   pthread_mutex_unlock(&lock->mutex);
   return 0;
}
