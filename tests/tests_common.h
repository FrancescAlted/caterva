/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <caterva.h>

#define mu_assert(message, test) do { if (!(test)) return message; } while (0)

#define mu_run_test(test) do \
    { char *message = test; tests_run++; \
      if (message) { printf("%c", 'F'); return message;} \
      else printf("%c", '.'); } while (0)

extern int tests_run;

#endif
