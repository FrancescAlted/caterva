/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#include "test_common.h"

void test_reshape(caterva_ctx *ctx, size_t ndim, size_t *shape_, size_t *pshape_,
                  size_t *pshape_dest_) {

    caterva_dims shape = caterva_new_dims(shape_, ndim);
    caterva_dims pshape = caterva_new_dims(pshape_, ndim);
    caterva_dims pshape_dest = caterva_new_dims(pshape_dest_, ndim);

    caterva_array *src = caterva_empty_array(ctx, NULL, pshape);

    size_t buf_size = 1;
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        buf_size *= (shape.dims[i]);
    }

    double *buf_src = (double *) malloc(buf_size * src->sc->typesize);
    fill_buf(buf_src, buf_size);

    caterva_from_buffer(src, shape, buf_src);

    caterva_array *dest = caterva_empty_array(ctx, NULL, pshape_dest);

    caterva_reshape(dest, src);

    double *buf_dest = (double *) malloc(dest->size * src->sc->typesize);

    caterva_to_buffer(dest, buf_dest);

    assert_buf(buf_src, buf_dest, dest->size, 1e-14);
    free(buf_src);
    free(buf_dest);
    caterva_free_array(src);
    caterva_free_array(dest);
}

LWTEST_DATA(reshape) {
    caterva_ctx *ctx;
};

LWTEST_SETUP(reshape) {
    data->ctx = caterva_new_ctx(NULL, NULL, BLOSC_CPARAMS_DEFAULTS, BLOSC_DPARAMS_DEFAULTS);
    data->ctx->cparams.typesize = sizeof(double);
}

LWTEST_TEARDOWN(reshape) {
    caterva_free_ctx(data->ctx);
}

LWTEST_FIXTURE(reshape, ndim2) {
    const size_t ndim = 2;
    size_t shape_[ndim] = {100, 100};
    size_t pshape_[ndim] = {10, 10};
    size_t pshape_dest_[ndim] = {20, 20};

    test_reshape(data->ctx, ndim, shape_, pshape_, pshape_dest_);
}

LWTEST_FIXTURE(reshape, ndim2_n) {
    const size_t ndim = 2;
    size_t shape_[ndim] = {8563, 8234};
    size_t pshape_[ndim] = {356, 353};
    size_t pshape_dest_[ndim] = {1033, 1033};

    test_reshape(data->ctx, ndim, shape_, pshape_, pshape_dest_);
}

LWTEST_FIXTURE(reshape, ndim6) {
    const size_t ndim = 6;
    size_t shape_[ndim] = {20, 15, 18, 14, 19, 20};
    size_t pshape_[ndim] = {2, 5, 4, 9, 4, 12};
    size_t pshape_dest_[ndim] = {3, 6, 8, 13, 15, 11};

    test_reshape(data->ctx, ndim, shape_, pshape_, pshape_dest_);
}