#include <stdio.h>
#include <stdlib.h>
#include "cli.h"

/* Строку в положительное целое; -1, если это не число или оно <= 0 */
static long parse_positive(const char* s) {
   char* end;
   long value = strtol(s, &end, 10);
   if (*end != '\0' || value <= 0) return -1;
   return value;
}

int cli_parse(int argc, char* argv[], workload_t* w) {
   if (argc != 3) return -1;

   long threads = parse_positive(argv[1]);
   w->lock      = rwlock_api_by_name(argv[2]);
   if (threads < 0 || w->lock == NULL) return -1;
   w->thread_count = (int) threads;

   printf("How many keys should be inserted in the main thread?\n");
   if (scanf("%ld", &w->initial_inserts) != 1 || w->initial_inserts < 0) return -1;
   printf("How many ops total should be executed?\n");
   if (scanf("%ld", &w->total_ops) != 1 || w->total_ops < 0) return -1;
   printf("Percent of ops that should be searches? (between 0 and 1)\n");
   if (scanf("%lf", &w->search_percent) != 1) return -1;
   printf("Percent of ops that should be inserts? (between 0 and 1)\n");
   if (scanf("%lf", &w->insert_percent) != 1) return -1;

   /* Доли от 0 до 1, и вместе не больше 1 (остальное — удаления) */
   if (w->search_percent < 0 || w->insert_percent < 0 ||
       w->search_percent + w->insert_percent > 1) return -1;
   return 0;
}

void cli_usage(const char* prog_name) {
   fprintf(stderr,
           "usage: %s <nthreads> <lib|my>\n"
           "  nthreads — количество потоков (> 0)\n"
           "  lib      — библиотечная pthread_rwlock_t\n"
           "  my       — своя my_rwlock_t\n"
           "Затем со стандартного ввода: число ключей в начальном списке,\n"
           "число операций, доля поисков и доля вставок (их сумма <= 1).\n",
           prog_name);
}
