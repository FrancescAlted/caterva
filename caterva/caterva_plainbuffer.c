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


#include <caterva.h>


int caterva_plainbuffer_free_array(caterva_array_t *carr) {

    if (carr->buf != NULL) {
        carr->ctx->free(carr->buf);
    }
    return 0;
}


int caterva_plainbuffer_append(caterva_array_t *carr, void *part, int64_t partsize) {
    if (carr->nparts == 0) {
        carr->buf = malloc(carr->size * (size_t) carr->ctx->cparams.typesize);
    } else {
        carr->nparts = 0;
    }
    int64_t start_[CATERVA_MAXDIM], stop_[CATERVA_MAXDIM];
    for (int i = 0; i < carr->ndim; ++i) {
        start_[i] = 0;
        stop_[i] = start_[i] + carr->pshape[i];
    }
    caterva_dims_t start = caterva_new_dims(start_, carr->ndim);
    caterva_dims_t stop = caterva_new_dims(stop_, carr->ndim);
    caterva_set_slice_buffer(carr, part, &start, &stop);

    return 0;
}


int caterva_plainbuffer_from_buffer(caterva_array_t *dest, caterva_dims_t *shape, const void *src) {
    caterva_append(dest, src, (size_t) dest->psize * dest->ctx->cparams.typesize);
    return 0;
}


int caterva_plainbuffer_to_buffer(caterva_array_t *src, void *dest) {
    memcpy(dest, src->buf, src->size * (size_t) src->ctx->cparams.typesize);
    return 0;
}


int caterva_plainbuffer_get_slice_buffer(void *dest, caterva_array_t *src, caterva_dims_t *start,
                                         caterva_dims_t *stop, caterva_dims_t *d_pshape) {
    uint8_t *bdest = dest;   // for allowing pointer arithmetic
    int64_t start_[CATERVA_MAXDIM];
    int64_t stop_[CATERVA_MAXDIM];
    int64_t d_pshape_[CATERVA_MAXDIM];
    int8_t s_ndim = src->ndim;

    caterva_dims_t shape = caterva_get_shape(src);
    int64_t s_shape[CATERVA_MAXDIM];
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        start_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = start->dims[i];
        stop_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = stop->dims[i];
        s_shape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = shape.dims[i];
        d_pshape_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = d_pshape->dims[i];
    }
    for (int j = 0; j < CATERVA_MAXDIM - s_ndim; ++j) {
        start_[j] = 0;
    }
    int64_t jj[CATERVA_MAXDIM];
    jj[7] = start_[7];
    for (jj[0] = start_[0]; jj[0] < stop_[0]; ++jj[0]) {
        for (jj[1] = start_[1]; jj[1] < stop_[1]; ++jj[1]) {
            for (jj[2] = start_[2]; jj[2] < stop_[2]; ++jj[2]) {
                for (jj[3] = start_[3]; jj[3] < stop_[3]; ++jj[3]) {
                    for (jj[4] = start_[4]; jj[4] < stop_[4]; ++jj[4]) {
                        for (jj[5] = start_[5]; jj[5] < stop_[5]; ++jj[5]) {
                            for (jj[6] = start_[6]; jj[6] < stop_[6]; ++jj[6]) {
                                int64_t chunk_pointer = 0;
                                int64_t chunk_pointer_inc = 1;
                                for (int i = CATERVA_MAXDIM - 1; i >= 0; --i) {
                                    chunk_pointer += jj[i] * chunk_pointer_inc;
                                    chunk_pointer_inc *= s_shape[i];
                                }
                                int64_t buf_pointer = 0;
                                int64_t buf_pointer_inc = 1;
                                for (int i = CATERVA_MAXDIM - 1; i >= 0; --i) {
                                    buf_pointer += (jj[i] - start_[i]) * buf_pointer_inc;
                                    buf_pointer_inc *= d_pshape_[i];
                                }
                                memcpy(&bdest[buf_pointer * src->ctx->cparams.typesize],
                                       &src->buf[chunk_pointer * src->ctx->cparams.typesize],
                                       (stop_[7] - start_[7]) * src->ctx->cparams.typesize);
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}


int caterva_plainbuffer_set_slice_buffer(caterva_array_t *dest, void *src, caterva_dims_t *start,
                                         caterva_dims_t *stop) {

    uint8_t *bsrc = src;   // for allowing pointer arithmetic
    int64_t start_[CATERVA_MAXDIM];
    int64_t stop_[CATERVA_MAXDIM];
    int8_t s_ndim = dest->ndim;

    caterva_dims_t shape = caterva_get_shape(dest);
    int64_t d_shape[CATERVA_MAXDIM];
    int64_t s_shape[CATERVA_MAXDIM];
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        start_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = start->dims[i];
        stop_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = stop->dims[i];
        d_shape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = (stop->dims[i] - start->dims[i]);
        s_shape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = shape.dims[i];
    }
    for (int j = 0; j < CATERVA_MAXDIM - s_ndim; ++j) {
        start_[j] = 0;
        d_shape[j] = 1;

    }
    int64_t jj[CATERVA_MAXDIM];
    jj[7] = start_[7];
    for (jj[0] = start_[0]; jj[0] < stop_[0]; ++jj[0]) {
        for (jj[1] = start_[1]; jj[1] < stop_[1]; ++jj[1]) {
            for (jj[2] = start_[2]; jj[2] < stop_[2]; ++jj[2]) {
                for (jj[3] = start_[3]; jj[3] < stop_[3]; ++jj[3]) {
                    for (jj[4] = start_[4]; jj[4] < stop_[4]; ++jj[4]) {
                        for (jj[5] = start_[5]; jj[5] < stop_[5]; ++jj[5]) {
                            for (jj[6] = start_[6]; jj[6] < stop_[6]; ++jj[6]) {
                                int64_t chunk_pointer = 0;
                                int64_t chunk_pointer_inc = 1;
                                for (int i = CATERVA_MAXDIM - 1; i >= 0; --i) {
                                    chunk_pointer += jj[i] * chunk_pointer_inc;
                                    chunk_pointer_inc *= s_shape[i];
                                }
                                int64_t buf_pointer = 0;
                                int64_t buf_pointer_inc = 1;
                                for (int i = CATERVA_MAXDIM - 1; i >= 0; --i) {
                                    buf_pointer += (jj[i] - start_[i]) * buf_pointer_inc;
                                    buf_pointer_inc *= d_shape[i];
                                }
                                memcpy(&dest->buf[chunk_pointer * dest->ctx->cparams.typesize],
                                       &bsrc[buf_pointer * dest->ctx->cparams.typesize],
                                       (stop_[7] - start_[7]) * dest->ctx->cparams.typesize);
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}


int caterva_plainbuffer_get_slice(caterva_array_t *dest, caterva_array_t *src,
                                  caterva_dims_t *start, caterva_dims_t *stop) {

    caterva_ctx_t *ctx = src->ctx;
    int typesize = ctx->cparams.typesize;

    uint64_t size = 1;
    for (int i = 0; i < stop->ndim; ++i) {
        size *= stop->dims[i] - start->dims[i];
    }
    dest->buf = malloc(size * typesize);
    caterva_dims_t shape = caterva_get_shape(dest);
    caterva_get_slice_buffer(dest->buf, src, start, stop, &shape);
    dest->filled = true;
    return 0;
}
