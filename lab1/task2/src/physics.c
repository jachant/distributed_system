#include <math.h>
#include "physics.h"
#include "vec3.h"

vec3_t physics_pair_force(const body_t* q, const body_t* k) {
   vec3_t d     = vec3_sub(k->pos, q->pos);           /* r_k - r_q       */
   double dist2 = vec3_dot(d, d);                     /* |r_k - r_q|^2   */
   double inv3  = 1.0 / (dist2 * sqrt(dist2));        /* 1/|r_k - r_q|^3 */
   return vec3_scale(d, GRAVITY_G * q->mass * k->mass * inv3);
}

/*
 * Формулы (8) из методички:
 *    x^n  = x^{n-1}  + v^{n-1} * dt
 *    v^n  = v^{n-1}  + F^{n-1} / m * dt
 * Координата сдвигается со СТАРОЙ скоростью, поэтому её обновляем первой.
 */
void physics_euler_step(body_t* body, vec3_t force, double dt) {
   body->pos = vec3_add(body->pos, vec3_scale(body->vel, dt));
   body->vel = vec3_add(body->vel, vec3_scale(force, dt / body->mass));
}
