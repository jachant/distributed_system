#ifndef VEC3_H
#define VEC3_H

#include "domain.h"

static inline vec3_t vec3_add(vec3_t a, vec3_t b) {
   return (vec3_t){ a.x + b.x, a.y + b.y, a.z + b.z };
}

static inline vec3_t vec3_sub(vec3_t a, vec3_t b) {
   return (vec3_t){ a.x - b.x, a.y - b.y, a.z - b.z };
}

static inline vec3_t vec3_scale(vec3_t a, double k) {
   return (vec3_t){ a.x * k, a.y * k, a.z * k };
}

static inline double vec3_dot(vec3_t a, vec3_t b) {
   return a.x * b.x + a.y * b.y + a.z * b.z;
}

#endif
