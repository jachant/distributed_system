#include "mandelbrot.h"

/* Главная кардиоида и круг |c + 1| <= 1/4 целиком лежат в множестве.
   Проверяем их сразу, чтобы не гонять для таких точек все итерации. */
static bool in_known_region(double complex c) {
   double x = creal(c), y = cimag(c);
   double dx = x - 0.25;
   double q  = dx * dx + y * y;
   bool in_cardioid = q * (q + dx) <= 0.25 * y * y;
   bool in_bulb     = (x + 1.0) * (x + 1.0) + y * y <= 0.0625;
   return in_cardioid || in_bulb;
}

bool mandelbrot_contains(double complex c) {
   if (in_known_region(c)) return true;

   double complex z = 0;                        /* z_1 = 0 */

   for (int n = 0; n < MANDELBROT_MAX_ITER; n++) {
      z = z * z + c; 

      /* |z| > 2  <=>  Re^2 + Im^2 > 4 (без вычисления корня).
         Строго '>': при c = -2 получается z = 2, 2, 2 ... — точка в множестве */
      double abs2 = creal(z) * creal(z) + cimag(z) * cimag(z);
      if (abs2 > 4.0) return false;
   }
   return true;
}
