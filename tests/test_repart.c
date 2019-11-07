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

static void test_reshape(caterva_ctx_t *ctx, int8_t ndim, int64_t *shape_, int64_t *pshape_,
                         int64_t *pshape_dest_) {

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

    if (caterva_repart(dest, src) == 0) {
        double *buf_dest = (double *) malloc((size_t) dest->size * src->ctx->cparams.typesize);

        caterva_to_buffer(dest, buf_dest);

        assert_buf(buf_src, buf_dest, (size_t) dest->size, 1e-14);
        free(buf_dest);
    }
    free(buf_src);
    caterva_free_array(src);
    caterva_free_array(dest);
}

LWTEST_DATA(repart) {
    caterva_ctx_t *ctx;
};

LWTEST_SETUP(repart) {
    data->ctx = caterva_new_ctx(NULL, NULL, BLOSC2_CPARAMS_DEFAULTS, BLOSC2_DPARAMS_DEFAULTS);
    data->ctx->cparams.typesize = sizeof(double);
}

LWTEST_TEARDOWN(repart) {
    caterva_free_ctx(data->ctx);
}

LWTEST_FIXTURE(repart, ndim2_plain) {
    const int8_t ndim = 2;
    int64_t shape_[] = {100, 100};

    test_reshape(data->ctx, ndim, shape_, NULL, NULL);
}

LWTEST_FIXTURE(repart, ndim2) {
    const int8_t ndim = 2;
    int64_t shape_[] = {8563, 8234};
    int64_t pshape_[] = {356, 353};
    int64_t pshape_dest_[] = {1033, 1033};

    test_reshape(data->ctx, ndim, shape_, pshape_, pshape_dest_);
}

LWTEST_FIXTURE(repart, ndim6) {
    const int8_t ndim = 6;
    int64_t shape_[] = {20, 15, 18, 14, 19, 20};
    int64_t pshape_[] = {2, 5, 4, 9, 4, 12};
    int64_t pshape_dest_[] = {3, 6, 8, 13, 15, 11};

    test_reshape(data->ctx, ndim, shape_, pshape_, pshape_dest_);
}
