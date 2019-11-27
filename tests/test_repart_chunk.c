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


static void test_repart_chunk(caterva_ctx_t *ctx, uint8_t ndim, int64_t *shape_, int64_t *pshape_, int64_t *spshape_) {

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
    for (int i = 0; i < carr->psize; ++i) {
        buffer_src[i] =  (double) i;
    }
    int res = caterva_repart_chunk((int8_t *) buffer_dest, buffer_src, carr, ctx);
    printf("%d", res);
    for (int i = 0; i < carr->psize; ++i) {
        printf("%f->", buffer_src[i]);
        printf("%f,", buffer_dest[i]);
    }


    free(buffer_src);
    free(buffer_dest);
    caterva_free_array(carr);
}

LWTEST_DATA(repart_chunk) {
    caterva_ctx_t *ctx;
};

LWTEST_SETUP(repart_chunk) {
    data->ctx = caterva_new_ctx(NULL, NULL, BLOSC2_CPARAMS_DEFAULTS, BLOSC2_DPARAMS_DEFAULTS);
    data->ctx->cparams.typesize = sizeof(double);
}

LWTEST_TEARDOWN(repart_chunk) {
    caterva_free_ctx(data->ctx);
}
/*
LWTEST_FIXTURE(repart_chunk, 2_dim) {
    const uint8_t ndim = 2;

    // sin padding (todo bonito)
    int64_t shape_[] = {8, 16};
    int64_t pshape_[] = {4, 8};
    int64_t spshape_[] = {2, 4};
    test_repart_chunk(data->ctx, ndim, shape_, pshape_, spshape_);

    //con padding
    int64_t shape__[] = {8, 16};
    int64_t pshape__[] = {4, 8};
    int64_t spshape__[] = {2, 3};
    test_repart_chunk(data->ctx, ndim, shape__, pshape__, spshape__);
}
*/
LWTEST_FIXTURE(repart_chunk, 3_dim) {
    const uint8_t ndim = 3;
    int64_t shape_[] = {4, 3, 2};
    int64_t pshape_[] = {2, 2, 2};
    int64_t spshape_[] = {1, 2, 1};


    test_repart_chunk(data->ctx, ndim, shape_, pshape_, spshape_);
}
/*

LWTEST_FIXTURE(repart_chunk, 4_dim_plain) {
    const uint8_t ndim = 4;
    int64_t shape_[] = {4, 3, 5, 4};

    test_repart_chunk(data->ctx, ndim, shape_, NULL, NULL);
}

LWTEST_FIXTURE(repart_chunk, 5_dim) {
    const uint8_t ndim = 5;
    int64_t shape_[] = {14, 23, 12, 11, 8};
    int64_t pshape_[] = {5, 12, 5, 3, 4};
    int64_t spshape_[] = {2, 4, 2, 1, 2};
    test_repart_chunk(data->ctx, ndim, shape_, pshape_, spshape_);
}
*/
