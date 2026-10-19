#ifndef SIMULATION_H
#define SIMULATION_H

#include "domain.h"

/*
 * Моделирует движение системы sys от t = 0 до config.t_end с шагом
 * не больше config.dt на config.thread_count потоках. Тела в sys
 * сдвигаются в конечное положение, траектории записываются в out.
 *
 * Возвращает 0 при успехе, -1 если не хватило памяти.
 * Память в out освобождается через trajectory_free.
 */
int  simulation_run(system_t* sys, config_t config, trajectory_t* out);

void trajectory_free(trajectory_t* traj);

#endif
