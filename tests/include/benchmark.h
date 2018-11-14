/*
 * Created by Aleix Alcacer
 * Parts of this source code is based on https://github.com/bvdberg/ctest/
 */

#ifndef LWTEST_BENCHMARK_H
#define LWTEST_BENCHMARK_H

#include <time.h>

#define LWTEST_GET_TIME(t) \
    lwtest_get_time(t, CLOCK_MONOTONIC)

#define LWTEST_GET_CPU_TIME(t) \
    lwtest_get_time(t, CLOCK_PROCESS_CPUTIME_ID)

int lwtest_get_time(double *t, clockid_t clk_id) {
    struct timespec ts;
    int err = clock_gettime(CLOCK_REALTIME, &ts);
    if(err != 0) return -1;
    *t = ts.tv_sec*1e9 + ts.tv_nsec;
    return 0;
}

#endif //LWTEST_BENCHMARK_H
