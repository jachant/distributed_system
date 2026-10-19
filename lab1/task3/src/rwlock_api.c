#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "rwlock_api.h"
#include "my_rwlock.h"

/* ---- библиотечная ---- */

static void* lib_create(void) {
   pthread_rwlock_t* lock = malloc(sizeof *lock);
   if (lock != NULL) pthread_rwlock_init(lock, NULL);
   return lock;
}
static void lib_destroy(void* lock) { pthread_rwlock_destroy(lock); free(lock); }
static void lib_rdlock(void* lock)  { pthread_rwlock_rdlock(lock); }
static void lib_wrlock(void* lock)  { pthread_rwlock_wrlock(lock); }
static void lib_unlock(void* lock)  { pthread_rwlock_unlock(lock); }

const rwlock_api_t LIB_RWLOCK = {
   "lib", lib_create, lib_destroy, lib_rdlock, lib_wrlock, lib_unlock
};

/* ---- своя ---- */

static void* my_create(void) {
   my_rwlock_t* lock = malloc(sizeof *lock);
   if (lock != NULL) my_rwlock_init(lock);
   return lock;
}
static void my_destroy(void* lock) { my_rwlock_destroy(lock); free(lock); }
static void my_rdlock(void* lock)  { my_rwlock_rdlock(lock); }
static void my_wrlock(void* lock)  { my_rwlock_wrlock(lock); }
static void my_unlock(void* lock)  { my_rwlock_unlock(lock); }

const rwlock_api_t MY_RWLOCK = {
   "my", my_create, my_destroy, my_rdlock, my_wrlock, my_unlock
};

const rwlock_api_t* rwlock_api_by_name(const char* name) {
   if (strcmp(name, LIB_RWLOCK.name) == 0) return &LIB_RWLOCK;
   if (strcmp(name, MY_RWLOCK.name) == 0)  return &MY_RWLOCK;
   return NULL;
}
