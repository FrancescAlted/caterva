//
// Created by Aleix Alcacer Sales on 07/06/2020.
//

#ifndef CATERVA_MINUNIT_H
#define CATERVA_MINUNIT_H

#ifdef __GNUC__
#define MU_UNUSED __attribute__((unused))
#else
#define MU_UNUSED
#endif

extern int tests_run;
extern int tests_failed;

#define MU_ASSERT(message, test) do { if (!(test)) return message; } while (0)
#define MU_RUN_TEST(test) do { printf("- %s... ", #test); char *message = test(); ++tests_run;\
                                if (message) { printf("[FAILED]\n"); ++tests_failed; \
                                } else {printf("[OK]\n"); } } while (0)
#define MU_RUN_SETUP(setup) do {setup();} while (0)
#define MU_RUN_TEARDOWN(teardown) do {teardown();} while (0)

#endif //CATERVA_MINUNIT_H
