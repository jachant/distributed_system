#include <stdio.h>
#include <stdlib.h>

#include "cli.h"
#include "mandelbrot.h"
#include "search.h"
#include "csv_writer.h"

#define OUTPUT_FILE "mandelbrot.csv"

static void print_report(config_t cfg, const search_result_t* r) {
   printf("Threads        = %d\n",  cfg.thread_count);
   printf("Points found   = %ld\n", cfg.point_count);
   printf("Points tested  = %ld\n", r->tested);
   printf("Max iterations = %d\n",  MANDELBROT_MAX_ITER);
   printf("Elapsed time   = %e seconds\n", r->elapsed);
}

int main(int argc, char* argv[]) {
   config_t config;
   if (cli_parse(argc, argv, &config) != 0) {
      cli_usage(argv[0]);
      return 1;
   }

   search_result_t result;
   if (search_parallel(config, MANDELBROT_REGION, &result) != 0) {
      fprintf(stderr, "Not enough memory\n");
      return 1;
   }

   print_report(config, &result);

   /* Пишем файл после замера времени, чтобы диск не влиял на измерения */
   int status = csv_write_points(OUTPUT_FILE, result.points, config.point_count);
   free(result.points);

   if (status != 0) {
      fprintf(stderr, "Cannot write %s\n", OUTPUT_FILE);
      return 1;
   }
   printf("Result written to %s\n", OUTPUT_FILE);
   return 0;
}
