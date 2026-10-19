#ifndef MY_RWLOCK_H
#define MY_RWLOCK_H

#include <stdbool.h>
#include <pthread.h>

typedef struct {
   pthread_mutex_t mutex;            /* защищает все поля ниже          */
   pthread_cond_t  readers_cv;       /* здесь ждут читатели             */
   pthread_cond_t  writers_cv;       /* здесь ждут писатели             */
   int             readers_active;   /* сколько потоков сейчас читают   */
   int             readers_waiting;  /* сколько ждут блокировку чтения  */
   int             writers_waiting;  /* сколько ждут блокировку записи  */
   bool            writer_active;    /* держит ли кто-то блокировку записи */
} my_rwlock_t;

/* Все функции возвращают 0, как и их аналоги из pthreads */
int my_rwlock_init(my_rwlock_t* lock);
int my_rwlock_destroy(my_rwlock_t* lock);
int my_rwlock_rdlock(my_rwlock_t* lock);
int my_rwlock_wrlock(my_rwlock_t* lock);
int my_rwlock_unlock(my_rwlock_t* lock);

#endif
