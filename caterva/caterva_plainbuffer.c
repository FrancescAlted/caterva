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
#include "caterva_utils.h"


int caterva_plainbuffer_array_free(caterva_ctx_t *ctx, caterva_array_t **array) {
    if ((*array)->buf != NULL) {
        ctx->cfg->free((*array)->buf);
    }
    return CATERVA_SUCCEED;
}

int caterva_plainbuffer_array_to_buffer(caterva_ctx_t *ctx, caterva_array_t *array, void *buffer,
                                        int64_t buffersize) {
    CATERVA_UNUSED_PARAM(ctx);
    CATERVA_UNUSED_PARAM(buffersize);
    memcpy(buffer, array->buf, (size_t) array->nitems * array->itemsize);
    return CATERVA_SUCCEED;
}

int caterva_plainbuffer_array_get_slice_buffer(caterva_ctx_t *ctx, caterva_array_t *array,
                                               int64_t *start, int64_t *stop, int64_t *shape,
                                               void *buffer, int64_t buffersize) {
    CATERVA_UNUSED_PARAM(ctx);
    CATERVA_UNUSED_PARAM(buffersize);

    uint8_t *buffer_b = (uint8_t *) buffer;
    int64_t buffer_start[CATERVA_MAX_DIM] = {0};

    if (array->ndim == 0) {
        memcpy(&buffer_b[0], &array->buf[0], array->itemsize);
        return CATERVA_SUCCEED;
    }

    CATERVA_ERROR(caterva_copy_buffer(array->ndim,
                            array->itemsize,
                            array->buf, array->shape, start, stop,
                            buffer, shape, buffer_start));

    return CATERVA_SUCCEED;
}

int caterva_plainbuffer_array_set_slice_buffer(caterva_ctx_t *ctx, void *buffer, int64_t buffersize,
                                               int64_t *start, int64_t *stop, int64_t *shape,
                                               caterva_array_t *array) {
    CATERVA_UNUSED_PARAM(ctx);
    CATERVA_UNUSED_PARAM(buffersize);

    uint8_t *buffer_b = (uint8_t *) buffer;
    int64_t buffer_start[CATERVA_MAX_DIM] = {0};

    if (array->ndim == 0) {
        memcpy(&array->buf[0], &buffer_b[0], array->itemsize);
        return CATERVA_SUCCEED;
    }
    int64_t buffer_stop[CATERVA_MAX_DIM] = {0};
    for (int i = 0; i < array->ndim; ++i) {
        buffer_stop[i] = buffer_start[i] + (stop[i] - start[i]);
    }

    CATERVA_ERROR(caterva_copy_buffer(array->ndim,
                                      array->itemsize,
                                      buffer, shape, buffer_start, buffer_stop,
                                      array->buf, array->shape, start));

    return CATERVA_SUCCEED;
}

int caterva_plainbuffer_array_get_slice(caterva_ctx_t *ctx, caterva_array_t *src, int64_t *start,
                                        int64_t *stop, caterva_storage_t *storage,
                                        caterva_array_t **array) {
    caterva_params_t params;
    params.ndim = src->ndim;
    params.itemsize = src->itemsize;
    for (int i = 0; i < src->ndim; ++i) {
        params.shape[i] = stop[i] - start[i];
    }

    CATERVA_ERROR(caterva_empty(ctx, &params, storage, array));

    if ((*array)->nitems == 0) {
        return CATERVA_SUCCEED;
    }

    CATERVA_ERROR(caterva_get_slice_buffer(ctx, src, start, stop, (*array)->buf, params.shape,
                                           (*array)->nitems * params.itemsize));

    return CATERVA_SUCCEED;
}

int caterva_plainbuffer_update_shape(caterva_array_t *array, int8_t ndim, int64_t *shape) {
    array->ndim = ndim;
    array->nitems = 1;
    array->extnitems = 1;
    array->chunknitems = 1;
    for (int i = 0; i < CATERVA_MAX_DIM; ++i) {
        if (i < ndim) {
            array->shape[i] = shape[i];
            array->extshape[i] = shape[i];
            array->chunkshape[i] = (int32_t)(shape[i]);
        } else {
            array->shape[i] = 1;
            array->extshape[i] = 1;
            array->chunkshape[i] = 1;
        }
        array->nitems *= array->shape[i];
        array->extnitems *= array->extshape[i];
        array->chunknitems *= array->chunkshape[i];
    }

    return CATERVA_SUCCEED;
}


int caterva_plainbuffer_array_squeeze_index(caterva_ctx_t *ctx,
                                            caterva_array_t *array,
                                            bool *index) {
    CATERVA_UNUSED_PARAM(ctx);
    uint8_t nones = 0;
    int64_t newshape[CATERVA_MAX_DIM];

    for (int i = 0; i < array->ndim; ++i) {
        if (index[i] == true) {
            if (array->shape[i] != 1) {
                CATERVA_ERROR(CATERVA_ERR_INVALID_INDEX);
            }
        } else {
            newshape[nones] = array->shape[i];
            nones += 1;
        }
    }

    CATERVA_ERROR(caterva_plainbuffer_update_shape(array, nones, newshape));

    return CATERVA_SUCCEED;
}


int caterva_plainbuffer_array_squeeze(caterva_ctx_t *ctx, caterva_array_t *array) {
    CATERVA_UNUSED_PARAM(ctx);
    bool index[CATERVA_MAX_DIM];

    for (int i = 0; i < array->ndim; ++i) {
        if (array->shape[i] != 1) {
            index[i] = false;
        } else {
            index[i] = true;
        }
    }
    CATERVA_ERROR(caterva_plainbuffer_array_squeeze_index(ctx, array, index));

    return CATERVA_SUCCEED;
}


int caterva_plainbuffer_array_copy(caterva_ctx_t *ctx, caterva_params_t *params,
                                   caterva_storage_t *storage, caterva_array_t *src,
                                   caterva_array_t **dest) {
    CATERVA_ERROR(caterva_empty(ctx, params, storage, dest));

    CATERVA_ERROR(
            caterva_to_buffer(ctx, src, (*dest)->buf, (*dest)->nitems * (*dest)->itemsize));
    return CATERVA_SUCCEED;
}

int caterva_plainbuffer_array_empty(caterva_ctx_t *ctx, caterva_params_t *params,
                                    caterva_storage_t *storage, caterva_array_t **array) {
    /* Create a caterva_array_t buffer */
    (*array) = (caterva_array_t *) ctx->cfg->alloc(sizeof(caterva_array_t));
    if ((*array) == NULL) {
        DEBUG_PRINT("Pointer is null");
        return CATERVA_ERR_NULL_POINTER;
    }
    (*array)->cfg = (caterva_config_t *) ctx->cfg->alloc(sizeof(caterva_config_t));
    memcpy((*array)->cfg, ctx->cfg, sizeof(caterva_config_t));

    (*array)->storage = storage->backend;
    (*array)->ndim = params->ndim;
    (*array)->itemsize = params->itemsize;

    int64_t *shape = params->shape;

    caterva_plainbuffer_update_shape(*array, params->ndim, shape);

    // The partition cache (empty initially)
    (*array)->chunk_cache.data = NULL;
    (*array)->chunk_cache.nchunk = -1;  // means no valid cache yet

    (*array)->sc = NULL;

    uint8_t *buf = ctx->cfg->alloc((size_t)(*array)->extnitems * params->itemsize);

    (*array)->buf = buf;

    return CATERVA_SUCCEED;
}

int caterva_plainbuffer_array_zeros(caterva_ctx_t *ctx, caterva_params_t *params,
                                    caterva_storage_t *storage, caterva_array_t **array) {
    caterva_empty(ctx, params, storage, array);

    memset((*array)->buf, 0, (*array)->nitems * (*array)->itemsize);
    return CATERVA_SUCCEED;
}

int caterva_plainbuffer_array_full(caterva_ctx_t *ctx, caterva_params_t *params,
                                    caterva_storage_t *storage, void *fill_value, caterva_array_t **array) {
    caterva_empty(ctx, params, storage, array);

    for (int i = 0; i < (*array)->nitems; ++i) {
        memcpy(&(*array)->buf[i * (*array)->itemsize], fill_value, (*array)->itemsize);
    }

    return CATERVA_SUCCEED;
}
