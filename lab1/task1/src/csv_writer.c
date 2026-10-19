#include <stdio.h>
#include "csv_writer.h"

int csv_write_points(const char* path, const double complex* points, long count) {
   FILE* f = fopen(path, "w");
   if (f == NULL) return -1;

   fprintf(f, "x,y\n");
   for (long i = 0; i < count; i++) {
      /* x = Re c, y = Im c. %.17g — точное значение double: у границы
         множества даже округление в 10-м знаке может поменять ответ */
      fprintf(f, "%.17g,%.17g\n", creal(points[i]), cimag(points[i]));
   }

   return fclose(f) == 0 ? 0 : -1;
}
