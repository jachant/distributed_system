#ifndef CLI_H
#define CLI_H

#include "domain.h"

/* Разбирает "./program <nthreads> <lib|my>" и читает со stdin:
   число ключей для начального заполнения, число операций,
   долю поисков и долю вставок. Возвращает 0 или -1 при ошибке. */
int  cli_parse(int argc, char* argv[], workload_t* w);

void cli_usage(const char* prog_name);

#endif
