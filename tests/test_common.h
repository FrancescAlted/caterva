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

#define CATERVA_TEST_ERROR(rc) do{if (rc != CATERVA_SUCCEED) {LWTEST_ERR("%s:%d  Caterva error", __FILE__, __LINE__);}} while(0)

static void fill_buf(uint8_t *buf, uint8_t itemsize, size_t buf_size) LW_ATT_UNUSED;
static void fill_buf(uint8_t *buf, uint8_t itemsize, size_t buf_size) {
    switch (itemsize) {
        case 8:
            for (size_t i = 0; i < buf_size; ++i) {
                ((double *) buf)[i] = (double) i;
            }
            break;
        case 4:
            for (size_t i = 0; i < buf_size; ++i) {
                ((float *) buf)[i] = (float) i;
            }
            break;
        case 2:
            for (size_t i = 0; i < buf_size; ++i) {
                ((uint16_t *) buf)[i] = (uint16_t ) i;
            }
            break;
        case 1:
            for (size_t i = 0; i < buf_size; ++i) {
                ((uint8_t *) buf)[i] = (uint8_t ) i;
            }
            break;
        default:CATERVA_TEST_ERROR(CATERVA_ERR_INVALID_ARGUMENT);
    }
}

static void assert_buf(const uint8_t *exp, const uint8_t *real, uint8_t itemsize, size_t size, double tol) LW_ATT_UNUSED;
static void assert_buf(const uint8_t *exp, const uint8_t *real, uint8_t itemsize, size_t size, double tol) {
    switch (itemsize) {
        case 8:
            for (size_t i = 0; i < size; ++i) {
                double a = ((double *) exp)[i];
                double b = ((double *) real)[i];
                LWTEST_ASSERT_ALMOST_EQUAL_DOUBLE(a, b, tol);
            }
            break;
        case 4:
            for (size_t i = 0; i < size; ++i) {
                double a = ((float *) exp)[i];
                double b = ((float *) real)[i];
                LWTEST_ASSERT_ALMOST_EQUAL_DOUBLE(a, b, tol);
            }
            break;
        case 2:
            for (size_t i = 0; i < size; ++i) {
                double a = ((uint16_t *) exp)[i];
                double b = ((uint16_t *) real)[i];
                LWTEST_ASSERT_ALMOST_EQUAL_DOUBLE(a, b, tol);
            }
            break;
        case 1:
            for (size_t i = 0; i < size; ++i) {
                double a = ((uint8_t *) exp)[i];
                double b = ((uint8_t *) real)[i];
                LWTEST_ASSERT_ALMOST_EQUAL_DOUBLE(a, b, tol);
            }
            break;
        default:
            CATERVA_TEST_ERROR(CATERVA_ERR_INVALID_ARGUMENT);
    }

}

#endif
