/*
 * Created by Aleix Alcacer
 *  Part of this source code is based on https://github.com/bvdberg/ctest/
 */

#ifndef LWTEST_MAIN_H
#define LWTEST_MAIN_H

#include "core.h"
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define MSG_SIZE 4096

static size_t lwtest_errorsize;
static char *lwtest_errormsg;
static char lwtest_errorbuffer[MSG_SIZE];
static jmp_buf lwtest_err;
static const char *suite_name;
typedef int (*lwtest_filter_func)(struct lwtest *);

LWTEST_TEST(suite, test) {}

static void vprint_errormsg(const char *fmt, va_list ap) LWTEST_IMPL_FORMAT_PRINTF(1, 0);

static void print_errormsg(const char *fmt, ...) LWTEST_IMPL_FORMAT_PRINTF(1, 2);

static void vprint_errormsg(const char *fmt, va_list ap) {
    // (v)snprintf returns the number that would have been written
    const int ret = vsnprintf(lwtest_errormsg, lwtest_errorsize, fmt, ap);
    if (ret < 0) {
        lwtest_errormsg[0] = 0x00;
    } else {
        const size_t size = (size_t) ret;
        const size_t s = (lwtest_errorsize <= size ? size - lwtest_errorsize : size);
        // lwtest_errorsize may overflow at this point
        lwtest_errorsize -= s;
        lwtest_errormsg += s;
    }
}

static void print_errormsg(const char *fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    vprint_errormsg(fmt, argp);
    va_end(argp);
}

static void msg_start(const char *title) {
    print_errormsg("  %s: ", title);
}

static void msg_end(void) {
    print_errormsg("\n");
}

void LWTEST_ERR(const char *fmt, ...) {
    va_list argp;
    msg_start("ERR");

    va_start(argp, fmt);
    vprint_errormsg(fmt, argp);
    va_end(argp);

    msg_end();
    longjmp(lwtest_err, 1);
}

static int suite_all(struct lwtest *t) {
    (void) t; // fix unused parameter warning
    return 1;
}

static int suite_filter(struct lwtest *t) {
    return strncmp(suite_name, t->ssname, strlen(suite_name)) == 0;
}

int lwtest_main(int argc, const char *argv[]) {
    static int total = 0;
    static int num_ok = 0;
    static int num_fail = 0;
    static int num_skip = 0;
    static int idx = 1;
    static lwtest_filter_func filter = suite_all;

    if (argc == 2) {
        suite_name = argv[1];
        filter = suite_filter;
    }

    struct lwtest *lwtest_begin = &_LWTEST_SNAME(suite, test);
    struct lwtest *lwtest_end = &_LWTEST_SNAME(suite, test);
    // find begin and end of section by comparing magics
    while (1) {
        struct lwtest *t = lwtest_begin - 1;
        if (t->magic != _LWTEST_MAGIC) break;
        lwtest_begin--;
    }
    while (1) {
        struct lwtest *t = lwtest_end + 1;
        if (t->magic != _LWTEST_MAGIC) break;
        lwtest_end++;
    }
    lwtest_end++;    // end after last one

    struct lwtest *test;
    for (test = lwtest_begin; test != lwtest_end; test++) {
        if (test == &_LWTEST_SNAME(suite, test)) continue;
        if (filter(test)) total++;
    }

    for (test = lwtest_begin; test != lwtest_end; test++) {
        if (test == &_LWTEST_SNAME(suite, test)) continue;
        if (filter(test)) {
            lwtest_errorbuffer[0] = 0;
            lwtest_errorsize = MSG_SIZE - 1;
            lwtest_errormsg = lwtest_errorbuffer;
            if (test->data) {
                printf("TEST [%d/%d] (%s->%s) ", idx, total, test->ssname, test->ttname);
            } else {
                printf("TEST [%d/%d] (%s) ", idx, total, test->ssname);
            }
            fflush(stdout);
            if (test->skip) {
                printf("[SKIPPED]\n");
                num_skip++;
            } else {

                int result = setjmp(lwtest_err);
                if (result == 0) {
                    if (test->setup && *test->setup) (*test->setup)(test->data);
                    if (test->data)
                        test->run(test->data);
                    else
                        test->run();
                    if (test->teardown && *test->teardown) (*test->teardown)(test->data);
                    printf("[OK] ");
                    num_ok++;
                } else {
                    printf("[FAIL] ");
                    num_fail++;
                }

                printf("\n");
                if (lwtest_errorsize != MSG_SIZE - 1) printf("%s", lwtest_errorbuffer);
            }
            printf("\n");
            idx++;
        }
    }

    printf("RESULTS: %d tests (%d ok, %d failed, %d skipped)", total, num_ok, num_fail, num_skip);

    return num_fail;
}

#endif //LWTEST_MAIN_H
