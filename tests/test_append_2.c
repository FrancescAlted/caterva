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

#include "test_common.h"

static void test_append_2(caterva_ctx_t *ctx, uint8_t ndim, int64_t *shape_, int64_t *pshape_, int64_t *spshape_, double *result) {

    caterva_dims_t shape = caterva_new_dims(shape_, ndim);
    caterva_array_t *src;
    if (pshape_ != NULL) {
        caterva_dims_t pshape = caterva_new_dims(pshape_, ndim);
        caterva_dims_t spshape = caterva_new_dims(spshape_, ndim);
        src = caterva_empty_array_2(ctx, NULL, &pshape, &spshape);
    } else {
        src = caterva_empty_array_2(ctx, NULL, NULL, NULL);
    }

    caterva_update_shape(src, &shape);

    /* Fill empty caterva_array_t with blocks */
    double *buffer = (double *) malloc(src->psize * src->ctx->cparams.typesize);
    double ind = 0;
    int res = 0;
    while ((res == 0) && (! src->filled)) {
        for (int i = 0; i < src->next_size; ++i) {
            buffer[i] = ind;
            ind++;
        }
        printf("\n buffer \n");
        for (int i = 0; i < src->next_size; ++i) {
            printf("%f,", buffer[i]);
        }
        res = caterva_append_2(src, buffer, src->next_size * src->ctx->cparams.typesize);
    }
    free(buffer);

    /* Fill dest array with caterva_array_t data */
    double *bufdest = (double *) malloc((size_t)src->size * src->ctx->cparams.typesize);
    caterva_to_buffer_2(src, bufdest);

    double *bufassert = (double *) malloc((size_t)80);
    printf("\n bufdest \n");
    for (int i = 0; i < 10; ++i) {
            printf("%f,", bufdest[i]);
            bufassert[i] = bufdest[i];
    }
    assert_buf(bufassert, result, (size_t)10, 1e-14);

    /* Free mallocs  */
    free(bufdest);
    free(buffer);
    free(bufassert);
    caterva_free_array(src);
}

LWTEST_DATA(append_2) {
    caterva_ctx_t *ctx;
};

LWTEST_SETUP(append_2) {
    data->ctx = caterva_new_ctx(NULL, NULL, BLOSC2_CPARAMS_DEFAULTS, BLOSC2_DPARAMS_DEFAULTS);
    data->ctx->cparams.typesize = sizeof(double);
}

LWTEST_TEARDOWN(append_2) {
    caterva_free_ctx(data->ctx);
}

LWTEST_FIXTURE(append_2, 2_dim_pad_sp) {
    const uint8_t ndim = 2;
    int64_t shape_[] = {8, 8};
    int64_t pshape_[] = {4, 4};
    int64_t spshape_[] = {3, 3};
    double result[80] = {0,1,2,3,16,17,18,19,4,5};
    test_append_2(data->ctx, ndim, shape_, pshape_, spshape_, result);
}

LWTEST_FIXTURE(append_2, 2_dim) {
    const uint8_t ndim = 2;
    int64_t shape_[] = {10, 10};
    int64_t pshape_[] = {3, 3};
    int64_t spshape_[] = {2, 2};
    double result[80] = {0,1,2,9,10,11,18,19,20,27};
    test_append_2(data->ctx, ndim, shape_, pshape_, spshape_, result);
}

LWTEST_FIXTURE(append_2, 3_dim) {
    const uint8_t ndim = 3;
    int64_t shape_[] = {4, 3, 3};
    int64_t pshape_[] = {2, 2, 2};
    int64_t spshape_[] = {1, 1, 1};
    double result[80] = {0,1,8,2,3,9,12,13,16,4};
    test_append_2(data->ctx, ndim, shape_, pshape_, spshape_, result);
}

LWTEST_FIXTURE(append_2, 4_dim) {
    const uint8_t ndim = 4;
    int64_t shape_[] = {4, 3, 4, 6};
    int64_t pshape_[] = {2, 3, 2, 3};
    int64_t spshape_[] = {1, 2, 1, 1};
    double result[80] = {0,1,2,36,37,38,3,4,5,39};
    test_append_2(data->ctx, ndim, shape_, pshape_, spshape_, result);
}

LWTEST_FIXTURE(append_2, 5_dim) {
    const uint8_t ndim = 5;
    int64_t shape_[] = {14, 23, 12, 11, 8};
    int64_t pshape_[] = {5, 12, 5, 3, 4};
    int64_t spshape_[] = {2, 4, 2, 1, 2};
    double result[80] = {0,1,2,3,3600,3601,3602,3603,4,5};
    test_append_2(data->ctx, ndim, shape_, pshape_, spshape_, result);
}
