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

static void test_squeeze(caterva_ctx_t *ctx, int8_t ndim, int64_t *shape_, int64_t *pshape_,
                         int64_t *pshape_dest_, int64_t *start_, int64_t *stop_) {

    caterva_dims_t shape = caterva_new_dims(shape_, ndim);
    caterva_dims_t start = caterva_new_dims(start_, ndim);
    caterva_dims_t stop = caterva_new_dims(stop_, ndim);

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

    double *buf_src = (double *) malloc(buf_size * src->ctx->cparams.typesize);
    fill_buf(buf_src, buf_size);

    caterva_from_buffer(src, &shape, buf_src);

    caterva_array_t *dest;
    if (pshape_dest_ != NULL) {
        caterva_dims_t pshape_dest = caterva_new_dims(pshape_dest_, ndim);
        dest = caterva_empty_array(ctx, NULL, &pshape_dest);
    } else {
        dest = caterva_empty_array(ctx, NULL, NULL);
    }
    caterva_get_slice(dest, src, &start, &stop);

    caterva_squeeze(dest);

    LWTEST_ASSERT_TRUE(src->ndim != dest->ndim);

    free(buf_src);
    caterva_free_array(src);
    caterva_free_array(dest);
}

LWTEST_DATA(squeeze) {
    caterva_ctx_t *ctx;
};

LWTEST_SETUP(squeeze) {
    data->ctx = caterva_new_ctx(NULL, NULL, BLOSC2_CPARAMS_DEFAULTS, BLOSC2_DPARAMS_DEFAULTS);
    data->ctx->cparams.typesize = sizeof(double);
}

LWTEST_TEARDOWN(squeeze) {
    caterva_free_ctx(data->ctx);
}

LWTEST_FIXTURE(squeeze, ndim3) {
    const int8_t ndim = 3;
    int64_t shape_[] = {100, 100, 100};
    int64_t pshape_[] = {10, 10, 10};
    int64_t pshape_dest_[] = {21, 1, 12};
    int64_t start_[] = {5, 20, 60};
    int64_t stop_[] = {23, 21, 99};

    test_squeeze(data->ctx, ndim, shape_, pshape_, pshape_dest_, start_, stop_);
}

LWTEST_FIXTURE(squeeze, ndim5_plain) {
    const int8_t ndim = 5;
    int64_t shape_[] = {22, 25, 31, 19, 31};
    int64_t start_[] = {1, 12, 3, 12, 6};
    int64_t stop_[] = {16, 21, 19, 13, 21};

    test_squeeze(data->ctx, ndim, shape_, NULL, NULL, start_, stop_);
}

LWTEST_FIXTURE(squeeze, ndim7) {
    const int8_t ndim = 7;
    int64_t shape_[] = {6, 8, 12, 6, 7, 6, 9};
    int64_t pshape_[] = {2, 3, 5, 2, 4, 3, 2};
    int64_t pshape_dest_[] = {1, 3, 3, 2, 1, 1, 5};
    int64_t start_[] = {5, 3, 3, 2, 1, 0, 4};
    int64_t stop_[] = {6, 8, 10, 5, 2, 1, 9};

    test_squeeze(data->ctx, ndim, shape_, pshape_, pshape_dest_, start_, stop_);
}
