#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include "../caterva/caterva.c"
#include "../partcompute/partcompute.c"


#define mu_assert(message, test) do { if (!(test)) return message; } while (0)

#define mu_run_test(test) do \
    { char *message = test; tests_run++; \
      if (message) { printf("%c", 'F'); return message;} \
      else printf("%c", '.'); } while (0)

extern int tests_run;

#endif
