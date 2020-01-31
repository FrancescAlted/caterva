/*
 * Copyright (c) 2018 Francesc Alted, Aleix Alcacer.
 * Copyright (C) 2019-present Blosc Development team <blosc@blosc.org>
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 */

#include "test_common.h"

static void test_copy(caterva_context_t *ctx, int8_t ndim, int64_t *shape_, int64_t *pshape_, int64_t *pshape2_) {

    caterva_dims_t shape = caterva_new_dims(shape_, ndim);

    caterva_array_t *src;
    if (pshape_ != NULL) {
        caterva_dims_t pshape = caterva_new_dims(pshape_, ndim);
        src = caterva_array_empty(ctx, NULL, &pshape);
    } else {
        src = caterva_array_empty(ctx, NULL, NULL);
    }

    size_t buf_size = 1;
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        buf_size *= (shape.dims[i]);
    }

    /* Create original data */
    double *bsrc = (double *) malloc(buf_size * sizeof(double));
    fill_buf(bsrc, buf_size);

    /* Fill empty caterva_array_t with original data */
    CATERVA_TEST_ERROR(caterva_array_from_buffer(src, &shape, bsrc));

   caterva_array_t *dest;
    if (pshape2_ != NULL) {
        caterva_dims_t pshape2 = caterva_new_dims(pshape2_, ndim);
        dest = caterva_array_empty(ctx, NULL, &pshape2);
    } else {
        dest = caterva_array_empty(ctx, NULL, NULL);
    }

    CATERVA_TEST_ERROR(caterva_copy(dest, src));
    double *bdest = (double *) malloc(buf_size * sizeof(double));

    CATERVA_TEST_ERROR(caterva_array_to_buffer(dest, bdest));

    assert_buf(bsrc, bdest, (size_t) dest->size, 1e-14);

    /* Free mallocs */
    free(bsrc);
    free(bdest);
    CATERVA_TEST_ERROR(caterva_free_array(src));
    CATERVA_TEST_ERROR(caterva_free_array(dest));
}

LWTEST_DATA(copy) {
    caterva_context_t *ctx;
};

LWTEST_SETUP(copy) {
    data->ctx = caterva_context_new(NULL, NULL, BLOSC2_CPARAMS_DEFAULTS, BLOSC2_DPARAMS_DEFAULTS);
    data->ctx->cparams.typesize = sizeof(double);
}

LWTEST_TEARDOWN(copy) {
    caterva_context_free(data->ctx);
}

LWTEST_FIXTURE(copy, 2d_blosc_blosc) {
    const int8_t ndim = 2;
    int64_t shape_[] = {40, 30};
    int64_t pshape_[] = {26, 17};
    int64_t pshape2_[] = {14, 24};

    test_copy(data->ctx, ndim, shape_, pshape_, pshape2_);
}

LWTEST_FIXTURE(copy, 3d_blosc_pbuff) {
    const int8_t ndim = 3;
    int64_t shape_[] = {134, 56, 204};
    int64_t pshape_[] = {26, 17, 34};
    int64_t *pshape2_ = NULL;

    test_copy(data->ctx, ndim, shape_, pshape_, pshape2_);
}

LWTEST_FIXTURE(copy, 4d_pbuff_blosc) {
    const int8_t ndim = 4;
    int64_t shape_[] = {4, 3, 8, 5};
    int64_t *pshape_ = NULL;
    int64_t pshape2_[] = {2, 2, 7, 6};

    test_copy(data->ctx, ndim, shape_, pshape_, pshape2_);
}

LWTEST_FIXTURE(copy, 5d_pbuff_pbuff) {
    const int8_t ndim = 5;
    int64_t shape_[] = {4, 3, 8, 5, 10};
    int64_t *pshape_ = NULL;
    int64_t *pshape2_ = NULL;

    test_copy(data->ctx, ndim, shape_, pshape_, pshape2_);
}
