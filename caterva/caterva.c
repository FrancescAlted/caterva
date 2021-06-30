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

#include "caterva_blosc.h"
#include "caterva_plainbuffer.h"

int caterva_ctx_new(caterva_config_t *cfg, caterva_ctx_t **ctx) {
    CATERVA_ERROR_NULL(cfg);
    CATERVA_ERROR_NULL(ctx);

    (*ctx) = (caterva_ctx_t *) cfg->alloc(sizeof(caterva_ctx_t));
    CATERVA_ERROR_NULL(ctx);
    if (!(*ctx)) {
        DEBUG_PRINT("Allocation fails");
        return CATERVA_ERR_NULL_POINTER;
    }

    (*ctx)->cfg = (caterva_config_t *) cfg->alloc(sizeof(caterva_config_t));
    CATERVA_ERROR_NULL((*ctx)->cfg);
    if (!(*ctx)->cfg) {
        DEBUG_PRINT("Allocation fails");
        return CATERVA_ERR_NULL_POINTER;
    }
    memcpy((*ctx)->cfg, cfg, sizeof(caterva_config_t));

    return CATERVA_SUCCEED;
}

int caterva_ctx_free(caterva_ctx_t **ctx) {
    CATERVA_ERROR_NULL(ctx);

    void (*auxfree)(void *) = (*ctx)->cfg->free;
    auxfree((*ctx)->cfg);
    auxfree(*ctx);

    return CATERVA_SUCCEED;
}

int caterva_empty(caterva_ctx_t *ctx, caterva_params_t *params,
                  caterva_storage_t *storage, caterva_array_t **array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(params);
    CATERVA_ERROR_NULL(storage);
    CATERVA_ERROR_NULL(array);

    if (storage->backend == CATERVA_STORAGE_BLOSC) {
        CATERVA_ERROR(caterva_blosc_array_empty(ctx, params, storage, array));
    } else {
        CATERVA_ERROR(caterva_plainbuffer_array_empty(ctx, params, storage, array));
    }

    return CATERVA_SUCCEED;
}

int caterva_zeros(caterva_ctx_t *ctx, caterva_params_t *params,
                  caterva_storage_t *storage, caterva_array_t **array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(params);
    CATERVA_ERROR_NULL(storage);
    CATERVA_ERROR_NULL(array);

    if (storage->backend == CATERVA_STORAGE_BLOSC) {
        CATERVA_ERROR(caterva_blosc_array_zeros(ctx, params, storage, array));
    } else {
        CATERVA_ERROR(caterva_plainbuffer_array_zeros(ctx, params, storage, array));
    }

    return CATERVA_SUCCEED;
}

int caterva_full(caterva_ctx_t *ctx, caterva_params_t *params,
                 caterva_storage_t *storage, void *fill_value, caterva_array_t **array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(params);
    CATERVA_ERROR_NULL(storage);
    CATERVA_ERROR_NULL(array);

    if (storage->backend == CATERVA_STORAGE_BLOSC) {
        CATERVA_ERROR(caterva_blosc_array_full(ctx, params, storage, fill_value, array));
    } else {
        CATERVA_ERROR(caterva_plainbuffer_array_full(ctx, params, storage, fill_value, array));
    }

    return CATERVA_SUCCEED;
}

int
caterva_from_schunk(caterva_ctx_t *ctx, blosc2_schunk *schunk, caterva_array_t **array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(schunk);
    CATERVA_ERROR_NULL(array);

    CATERVA_ERROR(caterva_blosc_from_schunk(ctx, schunk, array));
    if ((*array) == NULL) {
        DEBUG_PRINT("Error creating a caterva container from a frame");
        return CATERVA_ERR_NULL_POINTER;
    }

    return CATERVA_SUCCEED;
}

int caterva_from_serial_schunk(caterva_ctx_t *ctx, uint8_t *serial_schunk, int64_t len,
                               caterva_array_t **array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(serial_schunk);
    CATERVA_ERROR_NULL(array);

    CATERVA_ERROR(caterva_blosc_from_serial_schunk(ctx, serial_schunk, len, array));

    return CATERVA_SUCCEED;
}

int caterva_open(caterva_ctx_t *ctx, const char *urlpath, caterva_array_t **array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(urlpath);
    CATERVA_ERROR_NULL(array);

    CATERVA_ERROR(caterva_blosc_open(ctx, urlpath, array));

    return CATERVA_SUCCEED;
}

int caterva_free(caterva_ctx_t *ctx, caterva_array_t **array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(array);
    void (*free)(void *) = (*array)->cfg->free;

    free((*array)->cfg);
    if (*array) {
        switch ((*array)->storage) {
            case CATERVA_STORAGE_BLOSC:
                caterva_blosc_array_free(ctx, array);
                break;
            case CATERVA_STORAGE_PLAINBUFFER:
                caterva_plainbuffer_array_free(ctx, array);
                break;
        }
        free(*array);
    }
    return CATERVA_SUCCEED;
}

int caterva_from_buffer(caterva_ctx_t *ctx, void *buffer, int64_t buffersize,
                        caterva_params_t *params, caterva_storage_t *storage,
                        caterva_array_t **array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(params);
    CATERVA_ERROR_NULL(storage);
    CATERVA_ERROR_NULL(buffer);
    CATERVA_ERROR_NULL(array);

    CATERVA_ERROR(caterva_empty(ctx, params, storage, array));

    if (buffersize < (int64_t)(*array)->nitems * (*array)->itemsize) {
        CATERVA_ERROR(CATERVA_ERR_INVALID_ARGUMENT);
    }

    if ((*array)->nitems == 0) {
        return CATERVA_SUCCEED;
    }

    int64_t start[CATERVA_MAX_DIM] = {0};
    int64_t *stop = (*array)->shape;
    int64_t *shape = (*array)->shape;
    CATERVA_ERROR(caterva_set_slice_buffer(ctx, buffer, shape, buffersize, start, stop, *array));

    return CATERVA_SUCCEED;
}

int caterva_to_buffer(caterva_ctx_t *ctx, caterva_array_t *array, void *buffer,
                      int64_t buffersize) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(array);
    CATERVA_ERROR_NULL(buffer);

    if (buffersize < (int64_t) array->nitems * array->itemsize) {
        CATERVA_ERROR(CATERVA_ERR_INVALID_ARGUMENT);
    }

    if (array->nitems == 0) {
        return CATERVA_SUCCEED;
    }

    switch (array->storage) {
        case CATERVA_STORAGE_BLOSC:
            CATERVA_ERROR(caterva_blosc_array_to_buffer(ctx, array, buffer, buffersize));
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            CATERVA_ERROR(caterva_plainbuffer_array_to_buffer(ctx, array, buffer, 0));
            break;
        default:
            CATERVA_ERROR(CATERVA_ERR_INVALID_STORAGE);
    }

    return CATERVA_SUCCEED;
}

int caterva_get_slice_buffer(caterva_ctx_t *ctx,
                             caterva_array_t *array,
                             int64_t *start, int64_t *stop,
                             void *buffer, int64_t *buffershape, int64_t buffersize) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(array);
    CATERVA_ERROR_NULL(start);
    CATERVA_ERROR_NULL(stop);
    CATERVA_ERROR_NULL(buffershape);
    CATERVA_ERROR_NULL(buffer);

    int64_t size = array->itemsize;
    for (int i = 0; i < array->ndim; ++i) {
        if (stop[i] - start[i] > buffershape[i]) {
            DEBUG_PRINT("The buffer shape can not be smaller than the slice shape");
            return CATERVA_ERR_INVALID_ARGUMENT;
        }
        size *= buffershape[i];
    }

    if (array->nitems == 0) {
        return CATERVA_SUCCEED;
    }

    if (buffersize < size) {
        CATERVA_ERROR(CATERVA_ERR_INVALID_ARGUMENT);
    }

    switch (array->storage) {
        case CATERVA_STORAGE_BLOSC:
            CATERVA_ERROR(
                caterva_blosc_array_get_slice_buffer(ctx, array, start, stop, buffershape, buffer, buffersize));
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            CATERVA_ERROR(
                    caterva_plainbuffer_array_get_slice_buffer(ctx, array, start, stop, buffershape, buffer, buffersize));
            break;
        default:
            CATERVA_ERROR(CATERVA_ERR_INVALID_STORAGE);
    }

    return CATERVA_SUCCEED;
}

int caterva_set_slice_buffer(caterva_ctx_t *ctx,
                             void *buffer, int64_t *buffershape, int64_t buffersize,
                             int64_t *start, int64_t *stop,
                             caterva_array_t *array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(buffer);
    CATERVA_ERROR_NULL(start);
    CATERVA_ERROR_NULL(stop);
    CATERVA_ERROR_NULL(array);

    int64_t size = array->itemsize;
    for (int i = 0; i < array->ndim; ++i) {
        size *= stop[i] - start[i];
    }

    if (buffersize < size) {
        CATERVA_ERROR(CATERVA_ERR_INVALID_ARGUMENT);
    }

    if (array->nitems == 0) {
        return CATERVA_SUCCEED;
    }

    switch (array->storage) {
        case CATERVA_STORAGE_BLOSC:
            CATERVA_ERROR(caterva_blosc_array_set_slice_buffer(
                    ctx, buffer, size * array->itemsize, start, stop, buffershape, array));
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            CATERVA_ERROR(caterva_plainbuffer_array_set_slice_buffer(
                    ctx, buffer, size * array->itemsize, start, stop, buffershape, array));
            break;
        default:
            CATERVA_ERROR(CATERVA_ERR_INVALID_STORAGE);
    }

    return CATERVA_SUCCEED;
}

int caterva_get_slice(caterva_ctx_t *ctx, caterva_array_t *src, int64_t *start,
                      int64_t *stop, caterva_storage_t *storage, caterva_array_t **array) {
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

    switch (storage->backend) {
        case CATERVA_STORAGE_BLOSC:
            CATERVA_ERROR(caterva_blosc_array_get_slice(ctx, src, start, stop, storage, array));
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            CATERVA_ERROR(caterva_plainbuffer_array_get_slice(ctx, src, start, stop, storage, array));
            break;
        default:
            CATERVA_ERROR(CATERVA_ERR_INVALID_STORAGE);
    }

    return CATERVA_SUCCEED;
}

int caterva_squeeze(caterva_ctx_t *ctx, caterva_array_t *array) {
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

int caterva_squeeze_index(caterva_ctx_t *ctx, caterva_array_t *array, bool *index) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(array);

    switch (array->storage) {
        case CATERVA_STORAGE_BLOSC:
            CATERVA_ERROR(caterva_blosc_array_squeeze_index(ctx, array, index));
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            CATERVA_ERROR(caterva_plainbuffer_array_squeeze_index(ctx, array, index));
            break;
        default:
            CATERVA_ERROR(CATERVA_ERR_INVALID_STORAGE);
    }

    return CATERVA_SUCCEED;
}

int caterva_copy(caterva_ctx_t *ctx, caterva_array_t *src, caterva_storage_t *storage,
                 caterva_array_t **array) {
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


int caterva_remove(caterva_ctx_t *ctx, char *urlpath) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(urlpath);

    CATERVA_ERROR(caterva_blosc_remove(ctx, urlpath));

    return CATERVA_SUCCEED;
}
