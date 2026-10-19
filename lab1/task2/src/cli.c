#include <stdio.h>
#include <stdlib.h>
#include "cli.h"

/* Строку в положительное число; -1, если это не число или оно <= 0 */
static double parse_positive(const char* s) {
   char* end;
   double value = strtod(s, &end);
   if (end == s || *end != '\0' || !(value > 0)) return -1;  /* "abc", "1x", "0" */
   return value;
}

int cli_parse(int argc, char* argv[], config_t* config) {
   if (argc < 3 || argc > 5) return -1;

   config->t_end        = parse_positive(argv[1]);
   config->input_path   = argv[2];
   config->thread_count = 1;
   config->dt           = config->t_end / CLI_DEFAULT_STEPS;

   if (config->t_end < 0) return -1;

   if (argc >= 4) {
      double threads = parse_positive(argv[3]);
      if (threads < 0 || threads != (int) threads) return -1;   /* целое > 0 */
      config->thread_count = (int) threads;
   }
   if (argc == 5) {
      config->dt = parse_positive(argv[4]);
      if (config->dt < 0) return -1;
   }
   return 0;
}

void cli_usage(const char* prog_name) {
   fprintf(stderr,
           "usage: %s <tend> <input_file> [nthreads] [dt]\n"
           "  tend       — конечный момент времени, с (> 0)\n"
           "  input_file — n, затем n строк: m x y z vx vy vz\n"
           "  nthreads   — количество потоков (по умолчанию 1)\n"
           "  dt         — шаг по времени, с (по умолчанию tend / %d)\n",
           prog_name, CLI_DEFAULT_STEPS);
}
