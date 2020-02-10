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

static void test_to_buffer_2(caterva_ctx_t *ctx, int8_t ndim, int64_t *shape_, int64_t *pshape_, int64_t *spshape_,
                            double *result) {


    caterva_dims_t shape = caterva_new_dims(shape_, ndim);
    caterva_array_t *src;
    if (pshape_ != NULL) {
        caterva_dims_t pshape = caterva_new_dims(pshape_, ndim);
        caterva_dims_t spshape = caterva_new_dims(spshape_, ndim);
        src = caterva_empty_array_2(ctx, NULL, &pshape, &spshape);
    } else {
        src = caterva_empty_array_2(ctx, NULL, NULL, NULL);
    }
    int64_t buf_size = 1;
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        buf_size *= (shape.dims[i]);
    }

    double *buf_src = (double *) malloc((size_t)buf_size * src->ctx->cparams.typesize);
    for (int64_t i = 0; i < buf_size; ++i) {
        buf_src[i] = (double) i;
    }
    caterva_from_buffer_2(src, &shape, buf_src);

    double *buf_dest = (double *) malloc((size_t)buf_size * src->ctx->cparams.typesize);
    caterva_to_buffer_2(src, buf_dest);
    assert_buf(buf_dest, result, (size_t)src->size, 1e-14);
    free(buf_src);
    free(buf_dest);
    caterva_free_array(src);
}

LWTEST_DATA(to_buffer_2) {
    caterva_ctx_t *ctx;
};

LWTEST_SETUP(to_buffer_2) {
    data->ctx = caterva_new_ctx(NULL, NULL, BLOSC2_CPARAMS_DEFAULTS, BLOSC2_DPARAMS_DEFAULTS);
    data->ctx->cparams.typesize = sizeof(double);
}

LWTEST_TEARDOWN(to_buffer_2) {
    caterva_free_ctx(data->ctx);
}


LWTEST_FIXTURE(to_buffer_2, ndim_2) {
    const int8_t ndim = 2;
    int64_t shape_[] = {10, 10};
    int64_t pshape_[] = {2, 3};
    int64_t spshape_[] = {1, 2};

    int64_t buf_size = 1;
    for (int i = 0; i < ndim; ++i) {
        buf_size *= (shape_[i]);
    }
    double *result = (double *) malloc((size_t)buf_size * data->ctx->cparams.typesize);
    for (int64_t i = 0; i < buf_size; ++i) {
        result[i] = (double) i;
    }

    test_to_buffer_2(data->ctx, ndim, shape_, pshape_, spshape_, result);
}

LWTEST_FIXTURE(to_buffer_2, ndim_2_2) {
    const int8_t ndim = 2;
    int64_t shape_[] = {5, 6};
    int64_t pshape_[] = {3, 3};
    int64_t spshape_[] = {2, 2};
    int64_t buf_size = 1;
    for (int i = 0; i < ndim; ++i) {
        buf_size *= (shape_[i]);
    }
    double *result = (double *) malloc((size_t)buf_size * data->ctx->cparams.typesize);
    for (int64_t i = 0; i < buf_size; ++i) {
        result[i] = (double) i;
    }

    test_to_buffer_2(data->ctx, ndim, shape_, pshape_, spshape_, result);
}

LWTEST_FIXTURE(to_buffer_2, ndim_3_no_sp) {
    const int8_t ndim = 3;
    int64_t shape_[] = {10, 10, 10};
    int64_t pshape_[] = {3, 5, 2};
    int64_t spshape_[] = {3, 5, 2};
    int64_t buf_size = 1;
    for (int i = 0; i < ndim; ++i) {
        buf_size *= (shape_[i]);
    }
    double *result = (double *) malloc((size_t)buf_size * data->ctx->cparams.typesize);
    for (int64_t i = 0; i < buf_size; ++i) {
        result[i] = (double) i;
    }

    test_to_buffer_2(data->ctx, ndim, shape_, pshape_, spshape_, result);
}

LWTEST_FIXTURE(to_buffer_2, ndim_3) {
    const int8_t ndim = 3;
    int64_t shape_[] = {5, 6, 3};
    int64_t pshape_[] = {3, 3, 3};
    int64_t spshape_[] = {2, 2, 2};

    int64_t buf_size = 1;
    for (int i = 0; i < ndim; ++i) {
        buf_size *= (shape_[i]);
    }
    double *result = (double *) malloc((size_t)buf_size * data->ctx->cparams.typesize);
    for (int64_t i = 0; i < buf_size; ++i) {
        result[i] = (double) i;
    }

    test_to_buffer_2(data->ctx, ndim, shape_, pshape_, spshape_, result);
}

LWTEST_FIXTURE(to_buffer_2, ndim_5_no_sp) {
    const int8_t ndim = 5;
    int64_t shape_[] = {10, 10, 10, 10, 10};
    int64_t pshape_[] = {3, 5, 2, 4, 5};
    int64_t spshape_[] = {3, 5, 2, 4, 5};
    int64_t buf_size = 1;
    for (int i = 0; i < ndim; ++i) {
        buf_size *= (shape_[i]);
    }
    double *result = (double *) malloc((size_t)buf_size * data->ctx->cparams.typesize);
    for (int64_t i = 0; i < buf_size; ++i) {
        result[i] = (double) i;
    }

    test_to_buffer_2(data->ctx, ndim, shape_, pshape_, spshape_, result);
}

LWTEST_FIXTURE(to_buffer_2, ndim_6) {
    const int8_t ndim = 6;
    int64_t shape_[] = {10, 10, 10, 10, 10, 10};
    int64_t pshape_[] = {3, 4, 2, 2, 2, 3};
    int64_t spshape_[] = {3, 4, 2, 2, 2, 3};
    int64_t buf_size = 1;
    for (int i = 0; i < ndim; ++i) {
        buf_size *= (shape_[i]);
    }
    double *result = (double *) malloc((size_t)buf_size * data->ctx->cparams.typesize);
    for (int64_t i = 0; i < buf_size; ++i) {
        result[i] = (double) i;
    }

    test_to_buffer_2(data->ctx, ndim, shape_, pshape_, spshape_, result);
}

LWTEST_FIXTURE(to_buffer_2, ndim_7_no_sp) {
    const int8_t ndim = 7;
    int64_t shape_[] = {4, 5, 3, 4, 4, 5, 2};
    int64_t pshape_[] = {3, 4, 2, 2, 2, 3, 1};
    int64_t spshape_[] = {3, 4, 2, 2, 2, 3, 1};
    int64_t buf_size = 1;
    for (int i = 0; i < ndim; ++i) {
        buf_size *= (shape_[i]);
    }
    double *result = (double *) malloc((size_t)buf_size * data->ctx->cparams.typesize);
    for (int64_t i = 0; i < buf_size; ++i) {
        result[i] = (double) i;
    }

    test_to_buffer_2(data->ctx, ndim, shape_, pshape_, spshape_, result);
}

