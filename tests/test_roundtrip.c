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

static void test_roundtrip(caterva_ctx_t *ctx, int8_t ndim, int64_t *shape_, int64_t *pshape_) {

    caterva_dims_t shape = caterva_new_dims(shape_, ndim);

    caterva_array_t *src;
    if (pshape_ != NULL) {
        caterva_dims_t pshape = caterva_new_dims(pshape_, ndim);
        src = caterva_empty_array(ctx, NULL, &pshape);
    } else {
        src = caterva_empty_array(ctx, NULL, NULL);
    }

    size_t buf_size = 1;
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        buf_size *= (shape.dims[i]);
    }

    /* Create original data */
    double *bufsrc = (double *) malloc(buf_size * sizeof(double));
    fill_buf(bufsrc, buf_size);

    /* Fill empty caterva_array_t with original data */
    caterva_from_buffer(src, &shape, bufsrc);

    /* Fill dest array with caterva_array_t data */
    double *bufdest = (double *) malloc(buf_size * sizeof(double));
    caterva_to_buffer(src, bufdest);

    /* Testing */
    assert_buf(bufsrc, bufdest, buf_size, 1e-15);

    /* Free mallocs */
    free(bufsrc);
    free(bufdest);
    caterva_free_array(src);
}

LWTEST_DATA(roundtrip) {
    caterva_ctx_t *ctx;
};

LWTEST_SETUP(roundtrip) {
    data->ctx = caterva_new_ctx(NULL, NULL, BLOSC_CPARAMS_DEFAULTS, BLOSC_DPARAMS_DEFAULTS);
    data->ctx->cparams.typesize = sizeof(double);
}

LWTEST_TEARDOWN(roundtrip) {
    caterva_free_ctx(data->ctx);
}

LWTEST_FIXTURE(roundtrip, 3_dim_plain) {
    const int8_t ndim = 3;
    int64_t shape_[] = {4, 3, 3};

    test_roundtrip(data->ctx, ndim, shape_, NULL);
}

LWTEST_FIXTURE(roundtrip, 3_dim_2) {
    const int8_t ndim = 3;
    int64_t shape_[] = {134, 56, 204};
    int64_t pshape_[] = {26, 17, 34};

    test_roundtrip(data->ctx, ndim, shape_, pshape_);
}

LWTEST_FIXTURE(roundtrip, 4_dim) {
    const int8_t ndim = 4;
    int64_t shape_[] = {4, 3, 8, 5};
    int64_t pshape_[] = {2, 2, 3, 3};

    test_roundtrip(data->ctx, ndim, shape_, pshape_);
}

LWTEST_FIXTURE(roundtrip, 4_dim_2_plain) {
    const int8_t ndim = 4;
    int64_t shape_[] = {78, 85, 34, 56};

    test_roundtrip(data->ctx, ndim, shape_, NULL);
}

LWTEST_FIXTURE(roundtrip, 5_dim) {
    const int8_t ndim = 5;
    int64_t shape_[] = {4, 3, 8, 5, 10};
    int64_t pshape_[] = {2, 2, 3, 3, 4};

    test_roundtrip(data->ctx, ndim, shape_, pshape_);
}

LWTEST_FIXTURE(roundtrip, 5_dim_2_plain) {
    const int8_t ndim = 5;
    int64_t shape_[] = {35, 55, 24, 36, 12};

    test_roundtrip(data->ctx, ndim, shape_, NULL);
}

LWTEST_FIXTURE(roundtrip, 6_dim) {
    const int8_t ndim = 6;
    int64_t shape_[] = {4, 3, 8, 5, 10, 12};
    int64_t pshape_[] = {2, 2, 3, 3, 4, 5};

    test_roundtrip(data->ctx, ndim, shape_, pshape_);
}

LWTEST_FIXTURE(roundtrip, 7_dim) {
    const int8_t ndim = 7;
    int64_t shape_[] = {12, 15, 24, 16, 12, 8, 7};

    test_roundtrip(data->ctx, ndim, shape_, NULL);
}

LWTEST_FIXTURE(roundtrip, 8_dim) {
    const int8_t ndim = 8;
    int64_t shape_[] = {4, 3, 8, 5, 10, 12, 6, 4};
    int64_t pshape_[] = {3, 2, 3, 3, 4, 5, 4, 2};

    test_roundtrip(data->ctx, ndim, shape_, pshape_);
}
