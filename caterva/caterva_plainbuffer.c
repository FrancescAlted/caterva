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


int caterva_plainbuffer_array_free(caterva_context_t *ctx, caterva_array_t **array) {
    if ((*array)->buf != NULL) {
        ctx->cfg->free((*array)->buf);
    }
    return CATERVA_SUCCEED;
}


int caterva_plainbuffer_array_append(caterva_context_t *ctx, caterva_array_t *array, void *chunk, int64_t chunksize) {
    if (array->nparts == 0) {
        array->buf = ctx->cfg->alloc(array->size * array->itemsize);
        CATERVA_ERROR_NULL(array->buf);
    }

    int64_t start[CATERVA_MAXDIM], stop[CATERVA_MAXDIM];
    for (int i = 0; i < array->ndim; ++i) {
        start[i] = 0;
        stop[i] = start[i] + array->chunkshape[i];
    }

    CATERVA_ERROR(caterva_array_set_slice_buffer(ctx, chunk, chunksize, start, stop, array));

    return CATERVA_SUCCEED;
}


int caterva_plainbuffer_array_from_buffer(caterva_context_t *ctx, caterva_array_t *array, void *buffer, int64_t buffersize) {
    CATERVA_ERROR(caterva_array_append(ctx, array, buffer, buffersize));
    return CATERVA_SUCCEED;
}


int caterva_plainbuffer_array_to_buffer(caterva_context_t *ctx, caterva_array_t *array, void *buffer) {
    CATERVA_UNUSED_PARAM(ctx);
    memcpy(buffer, array->buf, array->size * array->itemsize);
    return CATERVA_SUCCEED;
}


int caterva_plainbuffer_array_get_slice_buffer(caterva_context_t *ctx, caterva_array_t *array, int64_t *start,
                                               int64_t *stop, int64_t *shape, void *buffer) {

    CATERVA_UNUSED_PARAM(ctx);

    int64_t start__[CATERVA_MAXDIM];
    int64_t stop__[CATERVA_MAXDIM];
    int64_t shape__[CATERVA_MAXDIM];
    int64_t shape2__[CATERVA_MAXDIM];

    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        start__[i] = (i < array->ndim) ? start[i] : 0;
        stop__[i] = (i < array->ndim) ? stop[i] : 1;
        shape__[i] = (i < array->ndim) ? array->shape[i] : 1;
        shape2__[i] = (i < array->ndim) ? shape[i] : 1;
    }

    uint8_t *bdest = buffer;   // for allowing pointer arithmetic
    int64_t start_[CATERVA_MAXDIM];
    int64_t stop_[CATERVA_MAXDIM];
    int64_t d_pshape_[CATERVA_MAXDIM];
    int8_t s_ndim = array->ndim;

    int64_t s_shape[CATERVA_MAXDIM];
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        start_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = start__[i];
        stop_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = stop__[i];
        s_shape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = shape__[i];
        d_pshape_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = shape2__[i];
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
                                memcpy(&bdest[buf_pointer * array->itemsize],
                                       &array->buf[chunk_pointer * array->itemsize],
                                       (stop_[7] - start_[7]) * array->itemsize);
                            }
                        }
                    }
                }
            }
        }
    }
    return CATERVA_SUCCEED;
}


int caterva_plainbuffer_array_set_slice_buffer(caterva_context_t *ctx, void *buffer, int64_t buffersize, int64_t *start,
                                               int64_t *stop, caterva_array_t *array) {

    CATERVA_UNUSED_PARAM(ctx);
    CATERVA_UNUSED_PARAM(buffersize);

    uint8_t *bbuffer = buffer;   // for allowing pointer arithmetic
    int64_t start_[CATERVA_MAXDIM];
    int64_t stop_[CATERVA_MAXDIM];
    int8_t s_ndim = array->ndim;

    int64_t d_shape[CATERVA_MAXDIM];
    int64_t s_shape[CATERVA_MAXDIM];
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        start_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = start[i];
        stop_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = stop[i];
        d_shape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = (stop[i] - start[i]);
        s_shape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = array->shape[i];
    }
    for (int j = 0; j < CATERVA_MAXDIM - s_ndim; ++j) {
        start_[j] = 0;
        stop_[j] = 1;
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
                                memcpy(&array->buf[chunk_pointer * array->itemsize],
                                       &bbuffer[buf_pointer * array->itemsize],
                                       (stop_[7] - start_[7]) * array->itemsize);
                            }
                        }
                    }
                }
            }
        }
    }
    return CATERVA_SUCCEED;
}


int caterva_plainbuffer_array_get_slice(caterva_context_t *ctx, caterva_array_t *src, int64_t *start, int64_t *stop,
                                        caterva_array_t *array) {

    int typesize = src->itemsize;

    uint64_t size = 1;
    for (int i = 0; i < src->ndim; ++i) {
        size *= stop[i] - start[i];
    }
    array->buf = malloc(size * typesize);
    CATERVA_ERROR(caterva_array_get_slice_buffer(ctx, src, start, stop, array->shape, array->buf, size * typesize));
    array->filled = true;
    array->empty = false;

    return CATERVA_SUCCEED;
}


int caterva_plainbuffer_update_shape(caterva_array_t *array, int8_t ndim, int64_t *shape) {
    array->ndim = ndim;
    array->size = 1;
    array->extendedesize = 1;
    array->chunksize = 1;
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        if (i < ndim) {
            array->shape[i] = shape[i];
            array->extendedshape[i] = shape[i];
            array->chunkshape[i] = (int32_t) (shape[i]);
        } else {
            array->shape[i] = 1;
            array->extendedshape[i] = 1;
            array->chunkshape[i] = 1;
        }
        array->size *= array->shape[i];
        array->extendedesize *= array->extendedshape[i];
        array->chunksize *= array->chunkshape[i];
    }

    return CATERVA_SUCCEED;
}


int caterva_plainbuffer_array_squeeze(caterva_context_t *ctx, caterva_array_t *array) {
    CATERVA_UNUSED_PARAM(ctx);

    uint8_t nones = 0;
    int64_t newshape[CATERVA_MAXDIM];
    for (int i = 0; i < array->ndim; ++i) {
        if (array->shape[i] != 1) {
            newshape[nones] = array->shape[i];
            nones += 1;
        }
    }

    CATERVA_ERROR(caterva_plainbuffer_update_shape(array, nones, newshape));

    return CATERVA_SUCCEED;
}


int caterva_plainbuffer_array_copy(caterva_context_t *ctx, caterva_params_t *params, caterva_storage_t *storage,
                                   caterva_array_t *src, caterva_array_t **dest) {

    CATERVA_ERROR(caterva_array_empty(ctx, params, storage, dest));

    (*dest)->buf = ctx->cfg->alloc((size_t) (*dest)->size * (*dest)->itemsize);
    CATERVA_ERROR(caterva_array_to_buffer(ctx, src, (*dest)->buf, (*dest)->size * (*dest)->itemsize));
    (*dest)->filled = true;

    return CATERVA_SUCCEED;
}


int caterva_plainbuffer_array_empty(caterva_context_t *ctx, caterva_params_t *params,
                                    caterva_storage_t *storage, caterva_array_t **array) {
    /* Create a caterva_array_t buffer */
    (*array) = (caterva_array_t *) ctx->cfg->alloc(sizeof(caterva_array_t));
    if ((*array) == NULL) {
        DEBUG_PRINT("Pointer is null");
        return CATERVA_ERR_NULL_POINTER;
    }

    (*array)->storage = storage->backend;
    (*array)->ndim = params->ndim;
    (*array)->itemsize = params->itemsize;

    int64_t *shape = params->shape;

    (*array)->size = 1;
    (*array)->chunksize = 1;
    (*array)->extendedesize = 1;

    for (int i = 0; i < params->ndim; ++i) {
        (*array)->shape[i] = shape[i];
        (*array)->chunkshape[i] = shape[i];
        (*array)->extendedshape[i] = shape[i];

        (*array)->size *= shape[i];
        (*array)->chunksize *= shape[i];
        (*array)->extendedesize *= shape[i];
    }

    for (int i = params->ndim; i < CATERVA_MAXDIM; ++i) {
        (*array)->shape[i] = 1;
        (*array)->chunkshape[i] = 1;
        (*array)->extendedshape[i] = 1;
    }

    // The partition cache (empty initially)
    (*array)->part_cache.data = NULL;
    (*array)->part_cache.nchunk = -1;  // means no valid cache yet

    (*array)->sc = NULL;

    uint8_t *buf = ctx->cfg->alloc((*array)->extendedesize * params->itemsize);

    (*array)->buf = buf;

    return CATERVA_SUCCEED;
}
