#ifndef RWLOCK_API_H
#define RWLOCK_API_H

typedef struct {
   const char* name;
   void* (*create)(void);          /* выделить и инициализировать; NULL — нет памяти */
   void  (*destroy)(void* lock);
   void  (*rdlock)(void* lock);
   void  (*wrlock)(void* lock);
   void  (*unlock)(void* lock);
} rwlock_api_t;

extern const rwlock_api_t LIB_RWLOCK;   /* pthread_rwlock_t */
extern const rwlock_api_t MY_RWLOCK;    /* my_rwlock_t      */

/* "lib" или "my" -> соответствующий интерфейс; NULL, если имя неизвестно */
const rwlock_api_t* rwlock_api_by_name(const char* name);

#endif
