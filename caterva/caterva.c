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

#include "caterva.h"
#include "caterva_blosc.h"
#include "caterva_plainbuffer.h"



int caterva_context_new(caterva_config_t *cfg, caterva_context_t **ctx) {
    CATERVA_ERROR_NULL(cfg);
    CATERVA_ERROR_NULL(ctx);

    (*ctx) = (caterva_context_t *) cfg->alloc(sizeof(caterva_context_t));
    if (!(*ctx)) {
        DEBUG_PRINT("Allocation fails");
        return CATERVA_ERR_NULL_POINTER;
    }

    (*ctx)->cfg = (caterva_config_t *) cfg->alloc(sizeof(caterva_config_t));
    if (!(*ctx)->cfg) {
        DEBUG_PRINT("Allocation fails");
        return CATERVA_ERR_NULL_POINTER;
    }
    memcpy((*ctx)->cfg, cfg, sizeof(caterva_config_t));

    return CATERVA_SUCCEED;
}


int caterva_context_free(caterva_context_t **ctx) {
    CATERVA_ERROR_NULL(ctx);

    void (*auxfree)(void *) = (*ctx)->cfg->free;
    auxfree((*ctx)->cfg);
    auxfree(*ctx);

    return CATERVA_SUCCEED;
}


int caterva_array_empty(caterva_context_t *ctx, caterva_params_t *params, caterva_storage_t *storage,
                        caterva_array_t **array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(params);
    CATERVA_ERROR_NULL(storage);
    CATERVA_ERROR_NULL(array);

    if (storage->backend == CATERVA_STORAGE_BLOSC) {
        CATERVA_ERROR(caterva_blosc_array_empty(ctx, params, storage, array));
    } else {
        CATERVA_ERROR(caterva_plainbuffer_array_empty(ctx, params, storage, array));
    }

    (*array)->filled = false;
    (*array)->empty = true;
    (*array)->nparts = 0;

    return CATERVA_SUCCEED;
}


int caterva_array_from_frame(caterva_context_t *ctx, blosc2_frame *frame, bool copy, caterva_array_t **array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(frame);
    CATERVA_ERROR_NULL(array);

    CATERVA_ERROR(caterva_blosc_from_frame(ctx, frame, copy, array));
    if ((*array) == NULL) {
        DEBUG_PRINT("Error creating a caterva container from a frame");
        return CATERVA_ERR_NULL_POINTER;
    }
    return CATERVA_SUCCEED;
}


int caterva_array_from_sframe(caterva_context_t *ctx, uint8_t *sframe, int64_t len, bool copy, caterva_array_t **array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(sframe);
    CATERVA_ERROR_NULL(array);

    CATERVA_ERROR(caterva_blosc_from_sframe(ctx, sframe, len, copy, array));

    return CATERVA_SUCCEED;
}


int caterva_array_from_file(caterva_context_t *ctx, const char *filename, bool copy, caterva_array_t **array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(filename);
    CATERVA_ERROR_NULL(array);

    CATERVA_ERROR(caterva_blosc_from_file(ctx, filename, copy, array));

    return CATERVA_SUCCEED;
}


int caterva_array_free(caterva_context_t *ctx, caterva_array_t **array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(array);

    if (*array) {
        switch ((*array)->storage) {
            case CATERVA_STORAGE_BLOSC:caterva_blosc_array_free(ctx, array);
                break;
            case CATERVA_STORAGE_PLAINBUFFER:caterva_plainbuffer_array_free(ctx, array);
                break;
        }
        ctx->cfg->free(*array);
    }
    return CATERVA_SUCCEED;
}


int caterva_array_append(caterva_context_t *ctx, caterva_array_t *array, void *chunk, int64_t chunksize) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(array);
    CATERVA_ERROR_NULL(chunk);

    if (array->filled) {
        CATERVA_ERROR(CATERVA_ERR_CONTAINER_FILLED);
    }
    switch (array->storage) {
        case CATERVA_STORAGE_BLOSC:
            if (chunksize != array->next_chunksize * array->itemsize) {
                CATERVA_ERROR(CATERVA_ERR_INVALID_ARGUMENT);
            }
            CATERVA_ERROR(caterva_blosc_array_append(ctx, array, chunk, chunksize));
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            CATERVA_ERROR(caterva_plainbuffer_array_append(ctx, array, chunk, chunksize));
            break;
        default:
            CATERVA_ERROR(CATERVA_ERR_INVALID_STORAGE);
    }

    array->nparts++;
    if (array->nparts == array->extendedsize / array->chunksize) {
        array->filled = true;
    }

    return CATERVA_SUCCEED;
}


int caterva_array_from_buffer(caterva_context_t *ctx, void *buffer, int64_t buffersize, caterva_params_t *params,
                              caterva_storage_t *storage, caterva_array_t **array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(params);
    CATERVA_ERROR_NULL(storage);
    CATERVA_ERROR_NULL(buffer);
    CATERVA_ERROR_NULL(array);

    CATERVA_ERROR(caterva_array_empty(ctx, params, storage, array));

    if (buffersize != (int64_t) (*array)->size * (*array)->itemsize) {
        CATERVA_ERROR(CATERVA_ERR_INVALID_ARGUMENT);
    }

    switch ((*array)->storage) {
        case CATERVA_STORAGE_BLOSC:
            CATERVA_ERROR(caterva_blosc_array_from_buffer(ctx, *array, buffer, buffersize));
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            CATERVA_ERROR(caterva_plainbuffer_array_from_buffer(ctx, *array, buffer, buffersize));
            break;
        default:
            CATERVA_ERROR(CATERVA_ERR_INVALID_STORAGE);
    }

    return CATERVA_SUCCEED;
}


int caterva_array_to_buffer(caterva_context_t *ctx, caterva_array_t *array, void *buffer, int64_t buffersize) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(array);
    CATERVA_ERROR_NULL(buffer);

    if (buffersize < (int64_t) array->size * array->itemsize) {
        CATERVA_ERROR(CATERVA_ERR_INVALID_ARGUMENT);
    }

    switch (array->storage) {
        case CATERVA_STORAGE_BLOSC:
            CATERVA_ERROR(caterva_blosc_array_to_buffer(ctx, array, buffer));
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            CATERVA_ERROR(caterva_plainbuffer_array_to_buffer(ctx, array, buffer));
            break;
        default:
            CATERVA_ERROR(CATERVA_ERR_INVALID_STORAGE);
    }

    return CATERVA_SUCCEED;
}


int caterva_array_get_slice_buffer(caterva_context_t *ctx, caterva_array_t *src, int64_t *start, int64_t *stop,
                                   int64_t *shape, void *buffer, int64_t buffersize) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(src);
    CATERVA_ERROR_NULL(start);
    CATERVA_ERROR_NULL(stop);
    CATERVA_ERROR_NULL(shape);
    CATERVA_ERROR_NULL(buffer);

    int64_t size = 1;
    for (int i = 0; i < src->ndim; ++i) {
        if (stop[i] - start[i] > shape[i]) {
            DEBUG_PRINT("The buffer shape can not be smaller than the slice shape");
            return CATERVA_ERR_INVALID_ARGUMENT;
        }
        size *= shape[i];
    }

    if (buffersize < size * src->itemsize) {
        CATERVA_ERROR(CATERVA_ERR_INVALID_ARGUMENT);
    }

    switch (src->storage) {
        case CATERVA_STORAGE_BLOSC:
            CATERVA_ERROR(caterva_blosc_array_get_slice_buffer(ctx, src, start, stop, shape, buffer));
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            CATERVA_ERROR(caterva_plainbuffer_array_get_slice_buffer(ctx, src, start, stop, shape, buffer));
            break;
        default:
            CATERVA_ERROR(CATERVA_ERR_INVALID_STORAGE);
    }

    return CATERVA_SUCCEED;
}


int caterva_array_set_slice_buffer(caterva_context_t *ctx, void *buffer, int64_t buffersize, int64_t *start,
                                   int64_t *stop, caterva_array_t *array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(buffer);
    CATERVA_ERROR_NULL(start);
    CATERVA_ERROR_NULL(stop);
    CATERVA_ERROR_NULL(array);

    int64_t size = 1;
    for (int i = 0; i < array->ndim; ++i) {
        size *= stop[i] - start[i];
    }

    if (buffersize < size * array->itemsize) {
        CATERVA_ERROR(CATERVA_ERR_INVALID_ARGUMENT);
    }

    switch (array->storage) {
        case CATERVA_STORAGE_BLOSC:
            CATERVA_ERROR(CATERVA_ERR_INVALID_STORAGE);
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            CATERVA_ERROR(caterva_plainbuffer_array_set_slice_buffer(ctx, buffer, size * array->itemsize,
                                                                     start, stop, array));
            break;
        default:
            CATERVA_ERROR(CATERVA_ERR_INVALID_STORAGE);
    }

    return CATERVA_SUCCEED;
}


int caterva_array_get_slice(caterva_context_t *ctx, caterva_array_t *src, int64_t *start, int64_t *stop,
                            caterva_storage_t *storage, caterva_array_t **array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(storage);
    CATERVA_ERROR_NULL(src);
    CATERVA_ERROR_NULL(start);
    CATERVA_ERROR_NULL(stop);
    CATERVA_ERROR_NULL(array);

    caterva_params_t params;
    params.ndim = src->ndim;
    params.itemsize = src->itemsize;
    for (int i = 0; i < src->ndim; ++i) {
        params.shape[i] = stop[i] - start[i];
    }

    CATERVA_ERROR(caterva_array_empty(ctx, &params, storage, array));

    switch ((*array)->storage) {
        case CATERVA_STORAGE_BLOSC:
            CATERVA_ERROR(caterva_blosc_array_get_slice(ctx, src, start, stop, *array));
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            CATERVA_ERROR(caterva_plainbuffer_array_get_slice(ctx, src, start, stop, *array));
            break;
        default:
            CATERVA_ERROR(CATERVA_ERR_INVALID_STORAGE);
    }

    return CATERVA_SUCCEED;
}


int caterva_array_squeeze(caterva_context_t *ctx, caterva_array_t *array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(array);

    switch (array->storage) {
        case CATERVA_STORAGE_BLOSC:
            CATERVA_ERROR(caterva_blosc_array_squeeze(ctx, array));
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            CATERVA_ERROR(caterva_plainbuffer_array_squeeze(ctx, array));
            break;
        default:
            CATERVA_ERROR(CATERVA_ERR_INVALID_STORAGE);
    }

    return CATERVA_SUCCEED;
}


int caterva_array_copy(caterva_context_t *ctx, caterva_array_t *src, caterva_storage_t *storage, caterva_array_t **array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(src);
    CATERVA_ERROR_NULL(storage);
    CATERVA_ERROR_NULL(array);

    caterva_params_t params;
    params.itemsize = src->itemsize;
    params.ndim = src->ndim;
    for (int i = 0; i < src->ndim; ++i) {
        params.shape[i] = src->shape[i];
    }

    switch (storage->backend) {
        case CATERVA_STORAGE_BLOSC:
            CATERVA_ERROR(caterva_blosc_array_copy(ctx, &params, storage, src, array));
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            CATERVA_ERROR(caterva_plainbuffer_array_copy(ctx, &params, storage, src, array));
            break;
        default:
            CATERVA_ERROR(CATERVA_ERR_INVALID_STORAGE);
    }

    return CATERVA_SUCCEED;
}
