/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <caterva.h>
#include "include/core.h"
#include "include/assert.h"

void fill_buf(double *buf, size_t buf_size) {
    for (size_t i = 0; i < buf_size; ++i) {
        buf[i] = (double) i;
    }
}

void assert_buf(const double *exp, const double *real, size_t size, double tol) {
    for (size_t i = 0; i < size; ++i) {
        double a = exp[i];
        double b = real[i];
        LWTEST_ASSERT_ALMOST_EQUAL_DOUBLE(a, b, tol);
    }
}

#endif
