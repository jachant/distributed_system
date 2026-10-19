#include <stdio.h>
#include "csv_writer.h"

int csv_write_trajectory(const char* path, const trajectory_t* traj) {
   FILE* f = fopen(path, "w");
   if (f == NULL) return -1;

   fprintf(f, "t");
   for (long k = 1; k <= traj->body_count; k++)
      fprintf(f, ",x%ld,y%ld", k, k);
   fprintf(f, "\n");

   for (long row = 0; row < traj->rows; row++) {
      fprintf(f, "%.10g", traj->t[row]);
      const double* xy = traj->xy + row * traj->body_count * 2;
      for (long k = 0; k < traj->body_count; k++)
         fprintf(f, ",%.10g,%.10g", xy[2 * k], xy[2 * k + 1]);
      fprintf(f, "\n");
   }

   return fclose(f) == 0 ? 0 : -1;
}
