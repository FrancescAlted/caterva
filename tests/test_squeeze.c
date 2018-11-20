/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#include "test_common.h"

void test_squeeze(caterva_ctx *ctx, size_t ndim, size_t *shape_, size_t *pshape_,
                  size_t *pshape_dest_, size_t *start_, size_t *stop_) {

    caterva_dims shape = caterva_new_dims(shape_, ndim);
    caterva_dims pshape = caterva_new_dims(pshape_, ndim);
    caterva_dims pshape_dest = caterva_new_dims(pshape_dest_, ndim);
    caterva_dims start = caterva_new_dims(start_, ndim);
    caterva_dims stop = caterva_new_dims(stop_, ndim);

    caterva_array *src = caterva_empty_array(ctx, NULL, pshape);

    size_t buf_size = 1;
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        buf_size *= (shape.dims[i]);
    }

    double *buf_src = (double *) malloc(buf_size * src->sc->typesize);
    fill_buf(buf_src, buf_size);

    caterva_from_buffer(src, shape, buf_src);

    caterva_array *dest = caterva_empty_array(ctx, NULL, pshape_dest);

    caterva_get_slice(dest, src, start, stop);

    caterva_squeeze(dest);

    size_t ndim_s;
    caterva_get_ndim(dest, &ndim_s);
    size_t *shape_s = (size_t *) malloc(ndim_s * sizeof(size_t));
    caterva_get_shape(dest, shape_s);

    printf("Dimensiones de dest: %d\n", (int) ndim_s);
    printf("Shape de dest: (");
    for (int j = 0; j < ndim_s - 1; ++j) {
        printf("%d, ", (int) shape_s[j]);
    }
    printf("%d)", (int) shape_s[ndim_s - 1]);
    free(buf_src);
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
    const size_t ndim = 3;
    size_t shape_[ndim] = {100, 100, 100};
    size_t pshape_[ndim] = {10, 10, 10};
    size_t pshape_dest_[ndim] = {21, 1, 12};
    size_t start_[ndim] = {5, 20, 60};
    size_t stop_[ndim] = {23, 21, 99};

    test_squeeze(data->ctx, ndim, shape_, pshape_, pshape_dest_, start_, stop_);
}