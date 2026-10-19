#include <stdio.h>
#include <stdlib.h>
#include "cli.h"

/* Строку в положительное целое; -1, если это не число или оно <= 0 */
static long parse_positive(const char* s) {
   char* end;
   long value = strtol(s, &end, 10);
   if (*end != '\0' || value <= 0) return -1;   /* "abc", "12x", "0", "-3" */
   return value;
}

int cli_parse(int argc, char* argv[], config_t* config) {
   if (argc != 3) return -1;

   long threads = parse_positive(argv[1]);
   long points  = parse_positive(argv[2]);
   if (threads < 0 || points < 0) return -1;

   config->thread_count = (int) threads;
   config->point_count  = points;
   return 0;
}

void cli_usage(const char* prog_name) {
   fprintf(stderr,
           "usage: %s <nthreads> <npoints>\n"
           "  nthreads — количество потоков (> 0)\n"
           "  npoints  — сколько точек множества найти (> 0)\n",
           prog_name);
}
