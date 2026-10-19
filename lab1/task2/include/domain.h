#ifndef DOMAIN_H
#define DOMAIN_H

/* Вектор в трёхмерном пространстве */
typedef struct {
   double x, y, z;
} vec3_t;

/* Материальная точка */
typedef struct {
   double mass;   /* масса, кг          */
   vec3_t pos;    /* радиус-вектор, м   */
   vec3_t vel;    /* скорость, м/с      */
} body_t;

/* Система из count материальных точек */
typedef struct {
   long    count;
   body_t* bodies;
} system_t;

/* Параметры запуска из командной строки */
typedef struct {
   double      t_end;          /* tend — конечный момент времени, с */
   const char* input_path;     /* файл с массами, координатами, скоростями */
   int         thread_count;   /* количество потоков                */
   double      dt;             /* шаг по времени, с                 */
} config_t;

/* Траектории: координаты x, y всех точек в каждый момент времени */
typedef struct {
   long    rows;         /* моментов времени: t_0 = 0, ..., t_steps = tend */
   long    body_count;
   double* t;            /* t[row]                                         */
   double* xy;           /* xy[(row * body_count + i) * 2 + 0/1] = x_i, y_i */
   double  elapsed;      /* время работы потоков, секунды                  */
} trajectory_t;

#endif
