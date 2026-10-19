#ifndef CLI_H
#define CLI_H

#include "domain.h"

/* Сколько шагов по времени делать, если dt не задан */
#define CLI_DEFAULT_STEPS 1000

/* Разбирает "./program tend input_file [nthreads] [dt]".
   Возвращает 0 при успехе, -1 при неверных аргументах. */
int  cli_parse(int argc, char* argv[], config_t* config);

void cli_usage(const char* prog_name);

#endif 
