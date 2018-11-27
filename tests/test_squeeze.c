/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#include "test_common.h"

void test_squeeze(caterva_ctx_t *ctx, uint64_t ndim, uint64_t *shape_, uint64_t *pshape_,
                  uint64_t *pshape_dest_, uint64_t *start_, uint64_t *stop_) {

    caterva_dims_t shape = caterva_new_dims(shape_, ndim);
    caterva_dims_t pshape = caterva_new_dims(pshape_, ndim);
    caterva_dims_t pshape_dest = caterva_new_dims(pshape_dest_, ndim);
    caterva_dims_t start = caterva_new_dims(start_, ndim);
    caterva_dims_t stop = caterva_new_dims(stop_, ndim);

    caterva_array_t *src = caterva_empty_array(ctx, NULL, pshape);

    uint64_t buf_size = 1;
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        buf_size *= (shape.dims[i]);
    }

    double *buf_src = (double *) malloc(buf_size * src->sc->typesize);
    fill_buf(buf_src, buf_size);

    caterva_from_buffer(src, shape, buf_src);

    caterva_array_t *dest = caterva_empty_array(ctx, NULL, pshape_dest);

    caterva_get_slice(dest, src, start, stop);
    
    free(buf_src);
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
    const uint64_t ndim = 3;
    uint64_t shape_[] = {100, 100, 100};
    uint64_t pshape_[] = {10, 10, 10};
    uint64_t pshape_dest_[] = {21, 1, 12};
    uint64_t start_[] = {5, 20, 60};
    uint64_t stop_[] = {23, 21, 99};

    test_squeeze(data->ctx, ndim, shape_, pshape_, pshape_dest_, start_, stop_);
}
