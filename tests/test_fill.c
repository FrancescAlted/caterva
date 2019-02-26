/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#include "test_common.h"

void test_fill(caterva_ctx_t *ctx, uint8_t ndim, uint64_t *shape_, uint64_t *pshape_, void *value) {

    caterva_dims_t shape = caterva_new_dims(shape_, ndim);
    caterva_dims_t pshape = caterva_new_dims(pshape_, ndim);

    caterva_array_t *src = caterva_empty_array(ctx, NULL, pshape);

    /* Fill empty caterva_array_t with value */
    caterva_fill(src, shape, value);

    /* Fill dest array with caterva_array_t data */
    double *bufdest = (double *) malloc(src->size * src->sc->typesize);
    caterva_to_buffer(src, bufdest);

    for (int i = 0; i < src->size; ++i) {
        LWTEST_ASSERT_ALMOST_EQUAL_DOUBLE(bufdest[i], *((double *) value), 1e-10);
    }

    /* Free mallocs */
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

LWTEST_FIXTURE(fill, 3_dim) {
    const uint8_t ndim = 3;
    uint64_t shape_[] = {4, 3, 3};
    uint64_t pshape_[] = {2, 2, 2};
    double value = 13.1265;
    test_fill(data->ctx, ndim, shape_, pshape_, &value);
}


LWTEST_FIXTURE(fill, 5_dim) {
    const uint8_t ndim = 5;
    uint64_t shape_[] = {14, 23, 12, 11, 8};
    uint64_t pshape_[] = {5, 12, 5, 3, 4};
    double value = 0.005;
    test_fill(data->ctx, ndim, shape_, pshape_, &value);
}