/*
 * Copyright (C) 2018 Francesc Alted, Aleix Alcacer.
 * Copyright (C) 2019-present Blosc Development team <blosc@blosc.org>
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 */

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <caterva.h>
#include "include/core.h"
#include "include/assert.h"

static void fill_buf(double *buf, size_t buf_size) {
    for (size_t i = 0; i < buf_size; ++i) {
        buf[i] = (double) i;
    }
}

static void assert_buf(const double *exp, const double *real, size_t size, double tol) {
    for (size_t i = 0; i < size; ++i) {
        double a = exp[i];
        double b = real[i];
        LWTEST_ASSERT_ALMOST_EQUAL_DOUBLE(a, b, tol);
    }
}

#endif
