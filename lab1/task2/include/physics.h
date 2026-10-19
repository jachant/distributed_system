#ifndef PHYSICS_H
#define PHYSICS_H

#include "domain.h"

/* Гравитационная постоянная, м^3 / (кг * с^2) */
#define GRAVITY_G 6.67430e-11

/* Сила, действующая на тело q со стороны тела k */
vec3_t physics_pair_force(const body_t* q, const body_t* k);

/* Один шаг явного метода Эйлера (формулы 8) для тела под действием
   суммарной силы force, посчитанной в момент t^{n-1} */
void physics_euler_step(body_t* body, vec3_t force, double dt);

#endif
