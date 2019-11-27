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
#include <string.h>
#include <assert.h>
#include "caterva_blosc.h"
#include "caterva_plainbuffer.h"


#define CATERVA_UNUSED_PARAM(x) ((void)(x))


caterva_ctx_t *caterva_new_ctx(void *(*c_alloc)(size_t), void (*c_free)(void *), blosc2_cparams cparams, blosc2_dparams dparams) {
    caterva_ctx_t *ctx;
    ctx = (caterva_ctx_t *) malloc(sizeof(caterva_ctx_t));
    if (ctx == NULL) {
        fprintf(stderr, "Error allocating caterva context");
        return NULL;
    }
    if (c_alloc == NULL) {
        ctx->alloc = malloc;
    } else {
        ctx->alloc = c_alloc;
    }
    if (c_free == NULL) {
        ctx->free = free;
    } else {
        ctx->free = c_free;
    }
    ctx->cparams = cparams;
    ctx->dparams = dparams;
    return ctx;
}


int caterva_free_ctx(caterva_ctx_t *ctx) {
    if (ctx != NULL) {
        free(ctx);
    }
    return 0;
}


caterva_dims_t caterva_new_dims(const int64_t *dims, int8_t ndim) {
    caterva_dims_t dims_s = CATERVA_DIMS_DEFAULTS;
    for (int i = 0; i < ndim; ++i) {
        dims_s.dims[i] = dims[i];
    }
    dims_s.ndim = ndim;
    return dims_s;
}


caterva_array_t *caterva_empty_array(caterva_ctx_t *ctx, blosc2_frame *frame, caterva_dims_t *pshape) {
    if (ctx == NULL) {
        fprintf(stderr, "Caterva context can not be NULL\n");
        return NULL;
    }
    caterva_array_t *carr;
    if (pshape != NULL) {
        carr = caterva_blosc_empty_array(ctx, frame, pshape);
    } else {
        carr = caterva_plainbuffer_empty_array(ctx, frame, pshape);
    }
    if (carr == NULL) {
        fprintf(stderr, "Error creating an empty caterva array\n");
        return NULL;
    }
    /* Copy context to caterva_array_t */
    carr->ctx = (caterva_ctx_t *) ctx->alloc(sizeof(caterva_ctx_t));
    if (carr->ctx == NULL) {
        fprintf(stderr, "Error allocating caterva context\n");
    }
    memcpy(&carr->ctx[0], &ctx[0], sizeof(caterva_ctx_t));

    carr->empty = true;
    carr->filled = false;
    carr->nparts = 0;

    return carr;
}


caterva_array_t *caterva_from_frame(caterva_ctx_t *ctx, blosc2_frame *frame, bool copy) {
    caterva_array_t *carr = caterva_blosc_from_frame(ctx, frame, copy);
    if (carr == NULL) {
        fprintf(stderr, "Error creating a caterva container from a frame\n");
        return NULL;
    }
    return carr;
}


caterva_array_t *caterva_from_sframe(caterva_ctx_t *ctx, uint8_t *sframe, int64_t len, bool copy) {
    caterva_array_t *carr = caterva_blosc_from_sframe(ctx, sframe, len, copy);
    if (carr == NULL) {
        fprintf(stderr, "Error creating a caterva container from a serialized frame\n");
        return NULL;
    }
    return carr;
}


caterva_array_t *caterva_from_file(caterva_ctx_t *ctx, const char *filename, bool copy) {
    caterva_array_t *carr = caterva_blosc_from_file(ctx, filename, copy);
    if (carr == NULL) {
        fprintf(stderr, "Error creating a caterva container from a file\n");
        return NULL;
    }
    return carr;
}


int caterva_free_array(caterva_array_t *carr) {
    switch (carr->storage) {
        case CATERVA_STORAGE_BLOSC:
            caterva_blosc_free_array(carr);
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            caterva_plainbuffer_free_array(carr);
            break;
    }
    void (*aux_free)(void *) = carr->ctx->free;
    caterva_free_ctx(carr->ctx);
    aux_free(carr);
    return 0;
}


int caterva_update_shape(caterva_array_t *carr, caterva_dims_t *shape) {
    int rc = CATERVA_SUCCEED;
    carr->empty = false;
    switch (carr->storage) {
        case CATERVA_STORAGE_BLOSC:
            rc = caterva_blosc_update_shape(carr, shape);
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            rc = caterva_plainbuffer_update_shape(carr, shape);
            break;
    }

    CATERVA_ERROR(rc, "Error updating shape");

    fail:
        return rc;
}


int caterva_append(caterva_array_t *carr, void *part, int64_t partsize) {
    int rc = CATERVA_SUCCEED;
    if (partsize != (int64_t) carr->psize * carr->ctx->cparams.typesize) {
        rc = CATERVA_ERR_INVALID_ARGUMENT;
        goto fail;
    }
    if (carr->filled) {
        rc = CATERVA_ERR_CONTAINER_FILLED;
        goto fail;
    }

    switch (carr->storage) {
        case CATERVA_STORAGE_BLOSC:
            rc = caterva_blosc_append(carr, part, partsize);
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            rc = caterva_plainbuffer_append(carr, part, partsize);
            break;
    }
    CATERVA_ERROR(rc, "Error appending data to a caterva container");

    carr->nparts++;
    if (carr->nparts == carr->esize / carr->psize) {
        carr->filled = true;
    }
    fail:
        return rc;
}


int caterva_from_buffer(caterva_array_t *dest, caterva_dims_t *shape, const void *src) {
    int rc = caterva_update_shape(dest, shape);
    CATERVA_ERROR(rc, "Error updating shape");

    switch (dest->storage) {
        case CATERVA_STORAGE_BLOSC:
            rc = caterva_blosc_from_buffer(dest, shape, src);
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            rc = caterva_plainbuffer_from_buffer(dest, shape, src);
            break;
    }
    CATERVA_ERROR(rc, "Error creating a caterva array from a buffer");

    fail:
        return rc;
}


int caterva_to_buffer(caterva_array_t *src, void *dest) {
    int rc = CATERVA_SUCCEED;
    switch (src->storage) {
        case CATERVA_STORAGE_BLOSC:
            rc = caterva_blosc_to_buffer(src, dest);
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            rc = caterva_plainbuffer_to_buffer(src, dest);
            break;
    }
    CATERVA_ERROR(rc, "Error converting a caterva container to a buffer");
    fail:
        return rc;
}


int caterva_get_slice_buffer(void *dest, caterva_array_t *src, caterva_dims_t *start,
                             caterva_dims_t *stop, caterva_dims_t *d_pshape) {
    int rc = CATERVA_SUCCEED;
    switch (src->storage) {
        case CATERVA_STORAGE_BLOSC:
            rc = caterva_blosc_get_slice_buffer(dest, src, start, stop, d_pshape);
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            rc = caterva_plainbuffer_get_slice_buffer(dest, src, start, stop, d_pshape);
            break;
    }
    CATERVA_ERROR(rc, "Error getting a slice buffer from a caterva container");
    fail:
        return rc;
}


int caterva_get_slice_buffer_no_copy(void **dest, caterva_array_t *src, caterva_dims_t *start,
                                     caterva_dims_t *stop, caterva_dims_t *d_pshape) {
    CATERVA_UNUSED_PARAM(d_pshape);
    int64_t start_[CATERVA_MAXDIM];
    int64_t stop_[CATERVA_MAXDIM];
    int8_t s_ndim = src->ndim;

    caterva_dims_t shape = caterva_get_shape(src);
    int64_t s_shape[CATERVA_MAXDIM];
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        start_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = start->dims[i];
        stop_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = stop->dims[i];
        s_shape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = shape.dims[i];
    }
    for (int j = 0; j < CATERVA_MAXDIM - s_ndim; ++j) {
        start_[j] = 0;
    }

    int64_t chunk_pointer = 0;
    int64_t chunk_pointer_inc = 1;
    for (int i = CATERVA_MAXDIM - 1; i >= 0; --i) {
        chunk_pointer += start_[i] * chunk_pointer_inc;
        chunk_pointer_inc *= s_shape[i];
    }
    *dest = &src->buf[chunk_pointer * src->ctx->cparams.typesize];

    return 0;
}


int caterva_set_slice_buffer(caterva_array_t *dest, void *src, caterva_dims_t *start,
                             caterva_dims_t *stop) {
    int rc = CATERVA_SUCCEED;
    switch (dest->storage) {
        case CATERVA_STORAGE_BLOSC:
            rc = CATERVA_ERR_INVALID_STORAGE;
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            rc = caterva_plainbuffer_set_slice_buffer(dest, src, start, stop);
            break;
    }
    CATERVA_ERROR(rc, "Error setting a slice buffer into a caterva container");
    fail:
        return rc;
}


int caterva_get_slice(caterva_array_t *dest, caterva_array_t *src, caterva_dims_t *start,
                                                                   caterva_dims_t *stop) {
    int rc;
    if (start->ndim != stop->ndim) {
        rc = CATERVA_ERR_INVALID_ARGUMENT;
        goto fail;
    }
    if (start->ndim != src->ndim) {
        rc = CATERVA_ERR_INVALID_ARGUMENT;
        goto fail;
    }

    int64_t shape_[CATERVA_MAXDIM];
    for (int i = 0; i < start->ndim; ++i) {
        shape_[i] = stop->dims[i] - start->dims[i];
    }
    for (int i = (int) start->ndim; i < CATERVA_MAXDIM; ++i) {
        shape_[i] = 1;
        start->dims[i] = 0;
    }
    caterva_dims_t shape = caterva_new_dims(shape_, start->ndim);
    rc  = caterva_update_shape(dest, &shape);
    CATERVA_ERROR(rc, "Error updating shape");

    switch (dest->storage) {
        case CATERVA_STORAGE_BLOSC:
            rc = caterva_blosc_get_slice(dest, src, start, stop);
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            rc = caterva_plainbuffer_get_slice(dest, src, start, stop);
            break;
    }
    CATERVA_ERROR(rc, "Error getting a slice from a caterva container");
    fail:
        return rc;
}


int caterva_squeeze(caterva_array_t *src) {
    int rc = CATERVA_SUCCEED;
    switch (src->storage) {
        case CATERVA_STORAGE_BLOSC:
            rc = caterva_blosc_squeeze(src);
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            rc = caterva_plainbuffer_squeeze(src);
            break;
    }
    CATERVA_ERROR(rc, "Error squeezing a caterva container");
    fail:
        return rc;
}


int caterva_copy(caterva_array_t *dest, caterva_array_t *src) {

    switch (dest->storage) {
        case CATERVA_STORAGE_BLOSC:
            caterva_blosc_copy(dest, src);
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            caterva_plainbuffer_copy(dest, src);
            break;
    }
    return 0;
}


caterva_dims_t caterva_get_shape(caterva_array_t *src){
    caterva_dims_t shape = caterva_new_dims(src->shape, src->ndim);
    return shape;
}


caterva_dims_t caterva_get_pshape(caterva_array_t *src) {
    caterva_dims_t pshape;
    for (int i = 0; i < src->ndim; ++i) {
        pshape.dims[i] = src->pshape[i];
    }
    pshape.ndim = src->ndim;
    return pshape;
}
