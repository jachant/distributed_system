#ifndef BARRIER_H
#define BARRIER_H

#include <pthread.h>

typedef struct {
   pthread_mutex_t mutex;
   pthread_cond_t  all_arrived;
   int             count;        /* сколько потоков должно прийти       */
   int             waiting;      /* сколько уже ждут                    */
   unsigned long   generation;   /* номер «прохода» через барьер        */
} barrier_t;

void barrier_init(barrier_t* b, int count);
void barrier_wait(barrier_t* b);
void barrier_destroy(barrier_t* b);

#endif
