#ifndef CLI_H
#define CLI_H

#include "domain.h"

/* Разбирает "./program nthreads npoints".
   Возвращает 0 при успехе, -1 при неверных аргументах. */
int  cli_parse(int argc, char* argv[], config_t* config);

void cli_usage(const char* prog_name);

#endif
