#ifndef INPUT_H
#define INPUT_H

#include "domain.h"

/* Читает систему из файла. Возвращает 0 при успехе, -1 при ошибке
   (причина печатается в stderr). Массив sys->bodies освобождается free(). */
int input_read_system(const char* path, system_t* sys);

#endif
