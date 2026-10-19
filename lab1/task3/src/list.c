#include <stdlib.h>
#include "list.h"

void list_init(list_t* list) {
   list->head = NULL;
}

void list_free(list_t* list) {
   list_node_t* curr = list->head;
   while (curr != NULL) {
      list_node_t* next = curr->next;
      free(curr);
      curr = next;
   }
   list->head = NULL;
}

bool list_member(const list_t* list, int value) {
   const list_node_t* curr = list->head;
   while (curr != NULL && curr->data < value)
      curr = curr->next;
   return curr != NULL && curr->data == value;
}

/* Найти место для value: *pred — последний узел с data < value
   (NULL, если такого нет), возвращается первый узел с data >= value */
static list_node_t* find(const list_t* list, int value, list_node_t** pred) {
   list_node_t* curr = list->head;
   *pred = NULL;
   while (curr != NULL && curr->data < value) {
      *pred = curr;
      curr  = curr->next;
   }
   return curr;
}

bool list_insert(list_t* list, int value) {
   list_node_t* pred;
   list_node_t* curr = find(list, value, &pred);
   if (curr != NULL && curr->data == value) return false;   /* уже есть */

   list_node_t* node = malloc(sizeof *node);
   if (node == NULL) return false;
   node->data = value;
   node->next = curr;
   if (pred == NULL) list->head = node;
   else              pred->next = node;
   return true;
}

bool list_delete(list_t* list, int value) {
   list_node_t* pred;
   list_node_t* curr = find(list, value, &pred);
   if (curr == NULL || curr->data != value) return false;   /* нет такого */

   if (pred == NULL) list->head = curr->next;
   else              pred->next = curr->next;
   free(curr);
   return true;
}

long list_size(const list_t* list) {
   long n = 0;
   for (const list_node_t* c = list->head; c != NULL; c = c->next) n++;
   return n;
}
