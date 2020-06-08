//
// Created by Aleix Alcacer Sales on 07/06/2020.
//

#ifndef CATERVA_TEST_COMMON_H
#define CATERVA_TEST_COMMON_H

#include <caterva.h>
#include "minunit/minunit.h"

#define MU_ASSERT_CATERVA(rc) MU_ASSERT(print_error(rc), rc == CATERVA_SUCCEED)

#define MU_ASSERT_BUFFER(a, b, buflen) MU_ASSERT("Buffers are not equals", mu_assert_buffer(a, b, buflen))

static bool mu_assert_buffer(void *a, void *b, int64_t buflen) MU_UNUSED;
static bool mu_assert_buffer(void *a, void *b, int64_t buflen) {
    int8_t * ab = (int8_t *) a;
    int8_t * bb = (int8_t *) b;
    for (int i = 0; i < buflen; ++i) {
        if (ab[i] != bb[i]) {
            return false;
        }
    }
    return true;
}

static bool fill_buf(uint8_t *buf, uint8_t itemsize, size_t buf_size) MU_UNUSED;
        static bool fill_buf(uint8_t *buf, uint8_t itemsize, size_t buf_size) {
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
        default:
            return false;
    }
    return true;
}

#endif //CATERVA_TEST_COMMON_H
