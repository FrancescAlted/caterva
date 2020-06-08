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

#include <ctype.h>

extern int tests_run;
extern int tests_failed;
extern int tests_skiped;

#define MU_RUN_TEST(test) do { printf("- %-50s ", #test); char *message = test(); ++tests_run;\
                                if (message) { printf("[FAILED]\n"); printf("%s\n", message); ++tests_failed; } else {printf("[OK]\n"); } } while (0)
#define MU_RUN_TEST_SKIP(test) do { printf("- %-50s ", #test); ++tests_run; ++tests_skiped; printf("[SKIPED]\n"); } while (0)
#define MU_RUN_SETUP(setup) do {setup();} while (0)
#define MU_RUN_TEARDOWN(teardown) do {teardown();} while (0)

#define MU_RUN_SUITE(name) int tests_run = 0; \
int tests_failed = 0; \
int main() { \
    printf("%s SUITE\n", name); \
    append_tests(); \
    int tests_ok = tests_run - tests_failed; \
    printf("RESULTS: %d tests (%d ok, %d failed)\n", tests_run, tests_ok, tests_failed); \
    return tests_failed; \
} \



#endif //CATERVA_MINUNIT_H
