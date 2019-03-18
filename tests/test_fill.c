/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#include "test_common.h"

void test_fill(caterva_ctx_t *ctx, uint8_t ndim, int64_t *shape_, int64_t *pshape_, void *value) {

    caterva_dims_t shape = caterva_new_dims(shape_, ndim);
    caterva_array_t *src;
    if (pshape_ != NULL) {
        caterva_dims_t pshape = caterva_new_dims(pshape_, ndim);
        src = caterva_empty_array(ctx, NULL, &pshape);
    } else {
        src = caterva_empty_array(ctx, NULL, NULL);
    }

    /* Fill empty caterva_array_t with value */
    caterva_fill(src, shape, value);

    /* Fill dest array with caterva_array_t data */
    double *bufdest = (double *) malloc((size_t)src->size * src->ctx->cparams.typesize);
    caterva_to_buffer(src, bufdest);

    for (int i = 0; i < src->size; ++i) {
        LWTEST_ASSERT_ALMOST_EQUAL_DOUBLE(bufdest[i], *((double *) value), 1e-10);
    }

    /* Free mallocs  */
    free(bufdest);
    caterva_free_array(src);
}

LWTEST_DATA(fill) {
    caterva_ctx_t *ctx;
};

LWTEST_SETUP(fill) {
    data->ctx = caterva_new_ctx(NULL, NULL, BLOSC_CPARAMS_DEFAULTS, BLOSC_DPARAMS_DEFAULTS);
    data->ctx->cparams.typesize = sizeof(double);
}

LWTEST_TEARDOWN(fill) {
    caterva_free_ctx(data->ctx);
}

LWTEST_FIXTURE(fill, 2_dim_plain) {
    const uint8_t ndim = 2;
    int64_t shape_[] = {4, 3};
    double value = 1.1265;
    test_fill(data->ctx, ndim, shape_, NULL, &value);
}

LWTEST_FIXTURE(fill, 3_dim) {
    const uint8_t ndim = 3;
    int64_t shape_[] = {4, 3, 3};
    int64_t pshape_[] = {2, 2, 2};
    double value = 13.1265;
    test_fill(data->ctx, ndim, shape_, pshape_, &value);
}

LWTEST_FIXTURE(fill, 4_dim_plain) {
    const uint8_t ndim = 4;
    int64_t shape_[] = {4, 3, 5, 4};
    double value = -1.4567;
    test_fill(data->ctx, ndim, shape_, NULL, &value);
}

LWTEST_FIXTURE(fill, 5_dim) {
    const uint8_t ndim = 5;
    int64_t shape_[] = {14, 23, 12, 11, 8};
    int64_t pshape_[] = {5, 12, 5, 3, 4};
    double value = 0.005;
    test_fill(data->ctx, ndim, shape_, pshape_, &value);
}
