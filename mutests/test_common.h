//
// Created by Aleix Alcacer Sales on 07/06/2020.
//

#ifndef CATERVA_TEST_COMMON_H
#define CATERVA_TEST_COMMON_H

#include <caterva.h>
#include "minunit/minunit.h"
#include "minunit/assert.h"

#define MU_ASSERT_CATERVA(rc) MU_ASSERT(print_error(rc), rc == CATERVA_SUCCEED)

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
