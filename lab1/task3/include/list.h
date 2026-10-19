#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

typedef struct list_node {
   int               data;
   struct list_node* next;
} list_node_t;

typedef struct {
   list_node_t* head;
} list_t;

void list_init(list_t* list);
void list_free(list_t* list);

/* true, если value есть в списке */
bool list_member(const list_t* list, int value);

/* Вставляет value на своё место по возрастанию.
   true — вставили, false — такое значение уже было. */
bool list_insert(list_t* list, int value);

/* Удаляет value. true — удалили, false — такого значения не было. */
bool list_delete(list_t* list, int value);

long list_size(const list_t* list);

#endif
