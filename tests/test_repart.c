/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#include "test_common.h"

void test_reshape(caterva_ctx_t *ctx, int8_t ndim, int64_t *shape_, int64_t *pshape_,
                  int64_t *pshape_dest_) {

    caterva_dims_t shape = caterva_new_dims(shape_, ndim);
    caterva_dims_t pshape = caterva_new_dims(pshape_, ndim);
    caterva_dims_t pshape_dest = caterva_new_dims(pshape_dest_, ndim);

    caterva_array_t *src = caterva_empty_array(ctx, NULL, pshape);

    size_t buf_size = 1;
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        buf_size *= (shape.dims[i]);
    }

    double *buf_src = (double *) malloc(buf_size * src->sc->typesize);
    fill_buf(buf_src, buf_size);

    caterva_from_buffer(src, shape, buf_src);

    caterva_array_t *dest = caterva_empty_array(ctx, NULL, pshape_dest);

    caterva_repart(dest, src);

    double *buf_dest = (double *) malloc((size_t)dest->size * src->sc->typesize);

    caterva_to_buffer(dest, buf_dest);

    assert_buf(buf_src, buf_dest, (size_t)dest->size, 1e-14);
    free(buf_src);
    free(buf_dest);
    caterva_free_array(src);
    caterva_free_array(dest);
}

LWTEST_DATA(reshape) {
    caterva_ctx_t *ctx;
};

LWTEST_SETUP(reshape) {
    data->ctx = caterva_new_ctx(NULL, NULL, BLOSC_CPARAMS_DEFAULTS, BLOSC_DPARAMS_DEFAULTS);
    data->ctx->cparams.typesize = sizeof(double);
}

LWTEST_TEARDOWN(reshape) {
    caterva_free_ctx(data->ctx);
}

LWTEST_FIXTURE(reshape, ndim2) {
    const int8_t ndim = 2;
    int64_t shape_[] = {100, 100};
    int64_t pshape_[] = {10, 10};
    int64_t pshape_dest_[] = {20, 20};

    test_reshape(data->ctx, ndim, shape_, pshape_, pshape_dest_);
}

LWTEST_FIXTURE(reshape, ndim2_n) {
    const int8_t ndim = 2;
    int64_t shape_[] = {8563, 8234};
    int64_t pshape_[] = {356, 353};
    int64_t pshape_dest_[] = {1033, 1033};

    test_reshape(data->ctx, ndim, shape_, pshape_, pshape_dest_);
}

LWTEST_FIXTURE(reshape, ndim6) {
    const int8_t ndim = 6;
    int64_t shape_[] = {20, 15, 18, 14, 19, 20};
    int64_t pshape_[] = {2, 5, 4, 9, 4, 12};
    int64_t pshape_dest_[] = {3, 6, 8, 13, 15, 11};

    test_reshape(data->ctx, ndim, shape_, pshape_, pshape_dest_);
}