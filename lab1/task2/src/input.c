#include <stdio.h>
#include <stdlib.h>
#include "input.h"

int input_read_system(const char* path, system_t* sys) {
   FILE* f = fopen(path, "r");
   if (f == NULL) {
      fprintf(stderr, "Cannot open %s\n", path);
      return -1;
   }

   long n;
   if (fscanf(f, "%ld", &n) != 1 || n <= 0) {
      fprintf(stderr, "%s: first line must be the number of bodies (> 0)\n", path);
      fclose(f);
      return -1;
   }

   body_t* bodies = malloc(n * sizeof(body_t));
   if (bodies == NULL) {
      fprintf(stderr, "Not enough memory for %ld bodies\n", n);
      fclose(f);
      return -1;
   }

   for (long k = 0; k < n; k++) {
      body_t* b = &bodies[k];
      int read = fscanf(f, "%lf %lf %lf %lf %lf %lf %lf",
                        &b->mass,
                        &b->pos.x, &b->pos.y, &b->pos.z,
                        &b->vel.x, &b->vel.y, &b->vel.z);
      if (read != 7) {
         fprintf(stderr, "%s: body %ld: expected 7 numbers "
                         "(m x y z vx vy vz)\n", path, k + 1);
         free(bodies);
         fclose(f);
         return -1;
      }
   }

   fclose(f);
   sys->count  = n;
   sys->bodies = bodies;
   return 0;
}
