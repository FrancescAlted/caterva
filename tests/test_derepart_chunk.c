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

static void test_derepart_chunk(caterva_ctx_t *ctx, uint8_t ndim, int64_t *shape_, int64_t *pshape_, int64_t *spshape_, double result[]) {

    caterva_dims_t shape = caterva_new_dims(shape_, ndim);
    caterva_array_t *carr;
    if (pshape_ != NULL) {
        caterva_dims_t pshape = caterva_new_dims(pshape_, ndim);
        caterva_dims_t spshape = caterva_new_dims(spshape_, ndim);
        carr = caterva_empty_array_2(ctx, NULL, &pshape, &spshape);
    } else {
        carr = caterva_empty_array_2(ctx, NULL, NULL, NULL);
    }

    caterva_update_shape(carr, &shape);

    /* Fill empty caterva_array_t with blocks */
    int size_src = carr->psize * ctx->cparams.typesize;
    int size_dest = carr->epsize * ctx->cparams.typesize;
    double *buffer_src = (double *) malloc(size_src);
    double *buffer_dest = (double *) malloc(size_dest);
    double *buffer_dereparted = (double *) malloc(size_src);
    for (int i = 0; i < carr->psize; ++i) {
        buffer_src[i] =  (double) i;
    }
    caterva_repart_chunk((int8_t *) buffer_dest, size_dest, buffer_src, size_src, carr, carr->ctx);
    caterva_derepart_chunk((int8_t *) buffer_dereparted, size_src, (int8_t *) buffer_dest, size_dest, carr, carr->ctx);
    /*
    printf("\n derepart chunk \n", NULL);
    for (int i = 0; i < carr->psize; ++i) {
        printf("%f,", buffer_dest[i]);
    }
    */
    int res = caterva_repart_chunk((int8_t *) buffer_dest, size_dest, buffer_dereparted, size_src, carr, carr->ctx);
    if (res != 0) {
        printf("Error code : %d\n", res);
    }


    assert_buf(buffer_dest, result, (size_t)carr->epsize, 1e-8);   // tam epsize*typesize????????????????
    free(buffer_src);
    free(buffer_dest);
    caterva_free_array(carr);
}

LWTEST_DATA(derepart_chunk) {
    caterva_ctx_t *ctx;
};

LWTEST_SETUP(derepart_chunk) {
    data->ctx = caterva_new_ctx(NULL, NULL, BLOSC2_CPARAMS_DEFAULTS, BLOSC2_DPARAMS_DEFAULTS);
    data->ctx->cparams.typesize = sizeof(double);
}

LWTEST_TEARDOWN(derepart_chunk) {
    caterva_free_ctx(data->ctx);
}


LWTEST_FIXTURE(derepart_chunk, 2_dim) {
    const uint8_t ndim = 2;
    // sin padding (todo bonito)
    int64_t shape_[] = {8, 16};
    int64_t pshape_[] = {4, 8};
    int64_t spshape_[] = {2, 4};
    double result[1024] = {0, 1, 2, 3, 8, 9, 10, 11, 4, 5, 6, 7, 12, 13,
        14, 15, 16, 17, 18, 19, 24, 25, 26, 27, 20, 21, 22, 23, 28, 29, 30, 31};
    test_derepart_chunk(data->ctx, ndim, shape_, pshape_, spshape_, result);
}

LWTEST_FIXTURE(derepart_chunk, 2_dim_pad) {
    const uint8_t ndim = 2;
    //con padding
    int64_t shape__[] = {8, 16};
    int64_t pshape__[] = {4, 8};
    int64_t spshape__[] = {2, 3};
    double result[1024] = {0, 1, 2, 8, 9, 10, 3, 4, 5, 11, 12, 13, 6, 7, 0, 14, 15, 0,
                           16, 17, 18, 24, 25, 26, 19, 20, 21, 27, 28, 29, 22, 23, 0, 30, 31, 0};
    test_derepart_chunk(data->ctx, ndim, shape__, pshape__, spshape__, result);
}


LWTEST_FIXTURE(derepart_chunk, 3_dim) {
    const uint8_t ndim = 3;
    // sin padding
    int64_t shape_[] = {4, 3, 2};
    int64_t pshape_[] = {2, 2, 2};
    int64_t spshape_[] = {1, 2, 1};
    double result[1024] = {0, 2, 1, 3, 4, 6, 5, 7};
    test_derepart_chunk(data->ctx, ndim, shape_, pshape_, spshape_, result);
}

LWTEST_FIXTURE(derepart_chunk, 3_dim_pad) {
    const uint8_t ndim = 3;
    // con padding
    int64_t shape_[] = {4, 3, 2};
    int64_t pshape_[] = {2, 3, 2};
    int64_t spshape_[] = {1, 2, 1};
    double result[1024] = {0, 2, 1, 3, 4, 0, 5, 0, 6, 8, 7, 9, 10, 0, 11, 0};
    test_derepart_chunk(data->ctx, ndim, shape_, pshape_, spshape_, result);
}


LWTEST_FIXTURE(derepart_chunk, 4_dim) {
    const uint8_t ndim = 4;
    // sin padding
    int64_t shape_[] = {4, 3, 5, 4};
    int64_t pshape_[] = {2, 2, 2, 2};
    int64_t spshape_[] = {1, 2, 1, 1};
    double result[1024] = {0, 4, 1, 5, 2, 6, 3, 7, 8, 12, 9, 13, 10, 14, 11, 15};
    test_derepart_chunk(data->ctx, ndim, shape_, pshape_, spshape_, result);
}

LWTEST_FIXTURE(derepart_chunk, 4_dim_pad) {
    const uint8_t ndim = 4;
    // con padding
    int64_t shape_[] = {4, 3, 2, 6};
    int64_t pshape_[] = {2, 3, 2, 2};
    int64_t spshape_[] = {1, 2, 1, 1};
    double result[1024] = {0, 4, 1, 5, 2, 6, 3, 7, 8, 0, 9, 0, 10, 0, 11, 0,
                           12, 16, 13, 17, 14, 18, 15, 19, 20, 0, 21, 0, 22, 0, 23, 0};
    test_derepart_chunk(data->ctx, ndim, shape_, pshape_, spshape_, result);
}


LWTEST_FIXTURE(derepart_chunk, 5_dim_plain) {
    const uint8_t ndim = 5;
    int64_t shape_[] = {14, 23, 12, 11, 8};
    int64_t pshape_[] = {2, 2, 2, 2, 2};
    int64_t spshape_[] = {1, 1, 1, 1, 1};
    double result[1024];
    for(double i = 0.0; i < 32; i++){
        result[(int) i] = i;
    }
    test_derepart_chunk(data->ctx, ndim, shape_, pshape_, spshape_, result);
}

