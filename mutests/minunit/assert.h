//
// Created by Aleix Alcacer Sales on 08/06/2020.
//

#ifndef CATERVA_ASSERT_H
#define CATERVA_ASSERT_H

#include "minunit.h"

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

#define MU_ASSERT_EQUAL(a, b) MU_ASSERT("Parameters are not equals", a == b)
#define MU_ASSERT_NOT_EQUAL(a, b) MU_ASSERT("Parameters are equals", a != b)

#endif //CATERVA_ASSERT_H
