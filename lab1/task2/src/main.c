#include <stdio.h>
#include <stdlib.h>

#include "cli.h"
#include "input.h"
#include "simulation.h"
#include "csv_writer.h"

#define OUTPUT_FILE "trajectories.csv"

static void print_report(config_t cfg, const trajectory_t* traj) {
   printf("Bodies         = %ld\n", traj->body_count);
   printf("Threads        = %d\n",  cfg.thread_count);
   printf("Steps          = %ld\n", traj->rows - 1);
   printf("dt             = %g s\n", traj->t[1] - traj->t[0]);
   printf("Elapsed time   = %e seconds\n", traj->elapsed);
}

int main(int argc, char* argv[]) {
   config_t config;
   if (cli_parse(argc, argv, &config) != 0) {
      cli_usage(argv[0]);
      return 1;
   }

   system_t sys;
   if (input_read_system(config.input_path, &sys) != 0)
      return 1;

   trajectory_t traj;
   int status = simulation_run(&sys, config, &traj);
   free(sys.bodies);
   if (status != 0) {
      fprintf(stderr, "Not enough memory: too many bodies or steps "
                      "(try a larger dt)\n");
      return 1;
   }

   print_report(config, &traj);

   /* Пишем файл после замера времени, чтобы диск не влиял на измерения */
   status = csv_write_trajectory(OUTPUT_FILE, &traj);
   trajectory_free(&traj);

   if (status != 0) {
      fprintf(stderr, "Cannot write %s\n", OUTPUT_FILE);
      return 1;
   }
   printf("Result written to %s\n", OUTPUT_FILE);
   return 0;
}
