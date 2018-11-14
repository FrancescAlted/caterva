/*
 * Created by Aleix Alcacer
 * Part of this source code is based on https://github.com/bvdberg/ctest/
 */

#ifndef LWTEST_ASSERT_H
#define LWTEST_ASSERT_H

#include "core.h"
#include <math.h>

#define LWTEST_ASSERT_EQUAL_INT(exp, real) \
    lwtest_assert_equal_int(exp, real, __FILE__, __LINE__)

#define LWTEST_ASSERT_EQUAL_FLOAT(exp, real) \
    lwtest_assert_equal_float(exp, real, __FILE__, __LINE__)

#define LWTEST_ASSERT_EQUAL_DOUBLE(exp, real) \
    lwtest_assert_equal_double(exp, real, __FILE__, __LINE__)

#define LWTEST_ASSERT_ALMOST_EQUAL_FLOAT(exp, real) \
    lwtest_assert_almost_equal_float(exp, real, __FILE__, __LINE__)

#define LWTEST_ASSERT_ALMOST_EQUAL_DOUBLE(exp, real, tol) \
    lwtest_assert_almost_equal_double(exp, real,tol, __FILE__, __LINE__)


void lwtest_assert_equal_int(int exp, int real, const char *caller, int line) {

    if (exp != real) {
        LWTEST_ERR("%s:%d  expected %d, got %d", caller, line, exp, real);
    }
}

void lwtest_assert_equal_double(double exp, double real, const char *caller, int line) {
    if (exp != real) {
        LWTEST_ERR("%s:%d  expected %f, got %f", caller, line, exp, real);
    }
}

void lwtest_assert_equal_float(float exp, float real, const char *caller, int line) {
    if (exp != real) {
        LWTEST_ERR("%s:%d  expected %f, got %f", caller, line, exp, real);
    }
}

void lwtest_assert_almost_equal_double(double exp, double real, double tol, const char *caller,
                                       int line) {
    double err_rel = fabs((exp - real) / real);
    if (err_rel > tol) {
        LWTEST_ERR("%s:%d  expected %f, got %f", caller, line, exp, real);
    }
}

void lwtest_assert_almost_equal_float(float exp, float real, double tol, const char *caller,
                                       int line) {
    double err_rel = fabs((double)(exp - real) / real);
    if (err_rel > tol) {
        LWTEST_ERR("%s:%d  expected %f, got %f", caller, line, exp, real);
    }
}

#endif //LWTEST_ASSERT_H
