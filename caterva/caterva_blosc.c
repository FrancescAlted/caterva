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
#include "assert.h"


// big <-> little-endian and store it in a memory position.  Sizes supported: 1, 2, 4, 8 bytes.
static void swap_store(void *dest, const void *pa, int size) {
    uint8_t* pa_ = (uint8_t*)pa;
    uint8_t* pa2_ = malloc((size_t)size);
    int i = 1;                    /* for big/little endian detection */
    char* p = (char*)&i;

    if (p[0] == 1) {
        /* little endian */
        switch (size) {
            case 8:
                pa2_[0] = pa_[7];
                pa2_[1] = pa_[6];
                pa2_[2] = pa_[5];
                pa2_[3] = pa_[4];
                pa2_[4] = pa_[3];
                pa2_[5] = pa_[2];
                pa2_[6] = pa_[1];
                pa2_[7] = pa_[0];
                break;
            case 4:
                pa2_[0] = pa_[3];
                pa2_[1] = pa_[2];
                pa2_[2] = pa_[1];
                pa2_[3] = pa_[0];
                break;
            case 2:
                pa2_[0] = pa_[1];
                pa2_[1] = pa_[0];
                break;
            case 1:
                pa2_[0] = pa_[0];
                break;
            default:
                fprintf(stderr, "Unhandled size: %d\n", size);
        }
    }
    memcpy(dest, pa2_, size);
    free(pa2_);
}


static int32_t serialize_meta(int8_t ndim, int64_t *shape, const int32_t *chunkshape, uint8_t **smeta) {
    // Allocate space for Caterva metalayer
    int32_t max_smeta_len = 1 + 1 + 1 + (1 + ndim * (1 + sizeof(int64_t))) + \
        (1 + ndim * (1 + sizeof(int32_t))) + (1 + ndim * (1 + sizeof(int32_t)));
    *smeta = malloc((size_t)max_smeta_len);
    uint8_t *pmeta = *smeta;

    // Build an array with 5 entries (version, ndim, shape, chunkshape, blockshape)
    *pmeta++ = 0x90 + 5;

    // version entry
    *pmeta++ = CATERVA_METALAYER_VERSION;  // positive fixnum (7-bit positive integer)
    assert(pmeta - *smeta < max_smeta_len);

    // ndim entry
    *pmeta++ = (uint8_t)ndim;  // positive fixnum (7-bit positive integer)
    assert(pmeta - *smeta < max_smeta_len);

    // shape entry
    *pmeta++ = (uint8_t)(0x90) + ndim;  // fix array with ndim elements
    for (int8_t i = 0; i < ndim; i++) {
        *pmeta++ = 0xd3;  // int64
        swap_store(pmeta, shape + i, sizeof(int64_t));
        pmeta += sizeof(int64_t);
    }
    assert(pmeta - *smeta < max_smeta_len);

    // chunkshape entry
    *pmeta++ = (uint8_t)(0x90) + ndim;  // fix array with ndim elements
    for (int8_t i = 0; i < ndim; i++) {
        *pmeta++ = 0xd2;  // int32
        swap_store(pmeta, chunkshape + i, sizeof(int32_t));
        pmeta += sizeof(int32_t);
    }
    assert(pmeta - *smeta <= max_smeta_len);

    // blockshape entry
    *pmeta++ = (uint8_t)(0x90) + ndim;  // fix array with ndim elements
    int32_t *blockshape = malloc(CATERVA_MAXDIM * sizeof(int32_t));
    for (int8_t i = 0; i < ndim; i++) {
        *pmeta++ = 0xd2;  // int32
        blockshape[i] = 0;  // FIXME: update when support for multidimensional bshapes would be ready
        // NOTE: we need to initialize the header so as to avoid false negatives in valgrind
        swap_store(pmeta, blockshape + i, sizeof(int32_t));
        pmeta += sizeof(int32_t);
    }
    free(blockshape);
    assert(pmeta - *smeta <= max_smeta_len);
    int32_t slen = (int32_t)(pmeta - *smeta);

    return slen;
}


static int32_t deserialize_meta(uint8_t *smeta, uint32_t smeta_len, int8_t *ndim, int64_t *shape, int32_t *chunkshape) {
    uint8_t *pmeta = smeta;

    // Check that we have an array with 5 entries (version, ndim, shape, chunkshape, blockshape)
    assert(*pmeta == 0x90 + 5);
    pmeta += 1;
    assert(pmeta - smeta < smeta_len);

    // version entry
    int8_t version = pmeta[0];  // positive fixnum (7-bit positive integer)
    assert (version <= CATERVA_METALAYER_VERSION);
    pmeta += 1;
    assert(pmeta - smeta < smeta_len);

    // ndim entry
    *ndim = pmeta[0];
    int8_t ndim_aux = *ndim;  // positive fixnum (7-bit positive integer)
    assert (ndim_aux <= CATERVA_MAXDIM);
    pmeta += 1;
    assert(pmeta - smeta < smeta_len);

    // shape entry
    // Initialize to ones, as required by Caterva
    for (int i = 0; i < CATERVA_MAXDIM; i++) shape[i] = 1;
    assert(*pmeta == (uint8_t)(0x90) + ndim_aux);  // fix array with ndim elements
    pmeta += 1;
    for (int8_t i = 0; i < ndim_aux; i++) {
        assert(*pmeta == 0xd3);   // int64
        pmeta += 1;
        swap_store(shape + i, pmeta, sizeof(int64_t));
        pmeta += sizeof(int64_t);
    }
    assert(pmeta - smeta < smeta_len);

    // chunkshape entry
    // Initialize to ones, as required by Caterva
    for (int i = 0; i < CATERVA_MAXDIM; i++) chunkshape[i] = 1;
    assert(*pmeta == (uint8_t)(0x90) + ndim_aux);  // fix array with ndim elements
    pmeta += 1;
    for (int8_t i = 0; i < ndim_aux; i++) {
        assert(*pmeta == 0xd2);  // int32
        pmeta += 1;
        swap_store(chunkshape + i, pmeta, sizeof(int32_t));
        pmeta += sizeof(int32_t);
    }
    assert(pmeta - smeta <= smeta_len);

    // blockshape entry
    // Initialize to ones, as required by Caterva
    // for (int i = 0; i < CATERVA_MAXDIM; i++) blockshape[i] = 1;
    assert(*pmeta == (uint8_t)(0x90) + ndim_aux);  // fix array with ndim elements
    pmeta += 1;
    for (int8_t i = 0; i < ndim_aux; i++) {
        assert(*pmeta == 0xd2);  // int32
        pmeta += 1;
        // swap_store(blockshape + i, pmeta, sizeof(int32_t));
        pmeta += sizeof(int32_t);
    }
    assert(pmeta - smeta <= smeta_len);
    uint32_t slen = (uint32_t)(pmeta - smeta);
    assert(slen == smeta_len);
    return 0;
}


int caterva_blosc_from_frame(caterva_context_t *ctx, blosc2_frame *frame, bool copy, caterva_array_t **array) {
    if (ctx == NULL) {
        DEBUG_PRINT("Context is null");
        return CATERVA_ERR_NULL_POINTER;
    }
    if (frame == NULL) {
        DEBUG_PRINT("Frame is null");
        return CATERVA_ERR_NULL_POINTER;
    }
    /* Create a caterva_array_t buffer */
    *array = (caterva_array_t *) ctx->cfg->alloc(sizeof(caterva_array_t));

    /* Create a schunk out of the frame */
    blosc2_schunk *sc = blosc2_schunk_from_frame(frame, copy);
    if (sc == NULL) {
        DEBUG_PRINT("Schunk is null");
        return CATERVA_ERR_NULL_POINTER;
    }
    (*array)->sc = sc;
    (*array)->storage = CATERVA_STORAGE_BLOSC;

    blosc2_cparams *cparams;
    if (blosc2_schunk_get_cparams(sc, &cparams) < 0) {
        DEBUG_PRINT("Blosc error");
        return CATERVA_ERR_NULL_POINTER;
    }

    (*array)->itemsize = cparams->typesize;

    // Deserialize the caterva metalayer
    uint8_t *smeta;
    uint32_t smeta_len;
    if (blosc2_get_metalayer(sc, "caterva", &smeta, &smeta_len) < 0) {
        DEBUG_PRINT("Blosc error");
        return CATERVA_ERR_BLOSC_FAILED;
    }
    deserialize_meta(smeta, smeta_len, &(*array)->ndim, (*array)->shape, (*array)->chunkshape);

    int64_t *shape = (*array)->shape;
    int32_t *chunkshape = (*array)->chunkshape;

    (*array)->size = 1;
    (*array)->chunksize = 1;
    (*array)->extendedesize = 1;

    for (int i = 0; i < (*array)->ndim; ++i) {
        if (shape[i] % chunkshape[i] == 0) {
            (*array)->extendedshape[i] = shape[i];
        } else {
            (*array)->extendedshape[i] = shape[i] + chunkshape[i] - shape[i] % chunkshape[i];
        }
        (*array)->size *= shape[i];
        (*array)->chunksize *= chunkshape[i];
        (*array)->extendedesize *= (*array)->extendedshape[i];
    }

    for (int i = (*array)->ndim; i < CATERVA_MAXDIM; ++i) {
        (*array)->shape[i] = 1;
        (*array)->chunkshape[i] = 1;
        (*array)->extendedshape[i] = 1;
    }


    // The partition cache (empty initially)
    (*array)->part_cache.data = NULL;
    (*array)->part_cache.nchunk = -1;  // means no valid cache yet

    (*array)->buf = NULL;

    if (sc->nchunks == (*array)->extendedesize / (*array)->chunksize) {
        (*array)->filled = true;
    } else {
        (*array)->filled = false;
    }

    return CATERVA_SUCCEED;
}


int caterva_blosc_from_sframe(caterva_context_t *ctx, uint8_t *sframe, int64_t len, bool copy, caterva_array_t **array) {
    // Generate a real frame first
    blosc2_frame *frame = blosc2_frame_from_sframe(sframe, len, copy);
    if (frame == NULL) {
        DEBUG_PRINT("Blosc error");
        return CATERVA_ERR_BLOSC_FAILED;
    }
    // ...and create a caterva array out of it
    CATERVA_ERROR(caterva_array_from_frame(ctx, frame, copy, array));

    if (copy) {
        // We don't need the frame anymore
        blosc2_free_frame(frame);
    }
    return CATERVA_SUCCEED;
}


int caterva_blosc_from_file(caterva_context_t *ctx, const char *filename, bool copy, caterva_array_t **array) {
    // Open the frame on-disk...
    blosc2_frame *frame = blosc2_frame_from_file(filename);
    if (frame == NULL) {
        DEBUG_PRINT("Blosc error");
        return CATERVA_ERR_BLOSC_FAILED;
    }
    // ...and create a caterva array out of it
    CATERVA_ERROR(caterva_array_from_frame(ctx, frame, copy, array));

    if (copy) {
        // We don't need the frame anymore
        blosc2_free_frame(frame);
    }
    return CATERVA_SUCCEED;
}


int caterva_blosc_array_free(caterva_context_t *ctx, caterva_array_t **array) {
    if ((*array)->sc != NULL) {
        blosc2_free_schunk((*array)->sc);
    }
    return CATERVA_SUCCEED;
}


int caterva_blosc_array_append(caterva_context_t *ctx, caterva_array_t *array, void *chunk, int64_t chunksize) {
    CATERVA_UNUSED_PARAM(ctx);

    if (blosc2_schunk_append_buffer(array->sc, chunk, chunksize) < 0) {
        CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
    }
    return CATERVA_SUCCEED;
}


int caterva_blosc_array_from_buffer(caterva_context_t *ctx, caterva_array_t *array, void *buffer, int64_t buffersize) {
    const int8_t *bbuffer = (int8_t *) buffer;
    
    int64_t d_shape[CATERVA_MAXDIM];
    int64_t d_eshape[CATERVA_MAXDIM];
    int32_t d_pshape[CATERVA_MAXDIM];
    int8_t d_ndim = array->ndim;

    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        d_shape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = array->shape[i];
        d_eshape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = array->extendedshape[i];
        d_pshape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = array->chunkshape[i];
    }

    int8_t typesize = array->itemsize;
    int8_t *chunk = ctx->cfg->alloc((size_t) array->chunksize * typesize);
    CATERVA_ERROR_NULL(chunk);
    
    /* Calculate the constants out of the for  */
    int64_t aux[CATERVA_MAXDIM];
    aux[7] = d_eshape[7] / d_pshape[7];
    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
        aux[i] = d_eshape[i] / d_pshape[i] * aux[i + 1];
    }

    /* Fill each chunk buffer */
    int64_t desp[CATERVA_MAXDIM];
    int32_t actual_psize[CATERVA_MAXDIM];
    for (int64_t ci = 0; ci < array->extendedesize / array->chunksize; ci++) {
        memset(chunk, 0, array->chunksize * typesize);
        /* Calculate the coord. of the chunk first element */
        desp[7] = ci % (d_eshape[7] / d_pshape[7]) * d_pshape[7];
        for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
            desp[i] = ci % (aux[i]) / (aux[i + 1]) * d_pshape[i];
        }
        /* Calculate if padding with 0s is needed for this chunk */
        for (int i = CATERVA_MAXDIM - 1; i >= 0; i--) {
            if (desp[i] + d_pshape[i] > d_shape[i]) {
                actual_psize[i] = d_shape[i] - desp[i];
            } else {
                actual_psize[i] = d_pshape[i];
            }
        }
        int32_t seq_copylen = actual_psize[7] * typesize;
        /* Copy each line of data from chunk to arr */
        int64_t ii[CATERVA_MAXDIM];
        for (ii[6] = 0; ii[6] < actual_psize[6]; ii[6]++) {
            for (ii[5] = 0; ii[5] < actual_psize[5]; ii[5]++) {
                for (ii[4] = 0; ii[4] < actual_psize[4]; ii[4]++) {
                    for (ii[3] = 0; ii[3] < actual_psize[3]; ii[3]++) {
                        for (ii[2] = 0; ii[2] < actual_psize[2]; ii[2]++) {
                            for (ii[1] = 0; ii[1] < actual_psize[1]; ii[1]++) {
                                for (ii[0] = 0; ii[0] < actual_psize[0]; ii[0]++) {
                                    int64_t d_a = d_pshape[7];
                                    int64_t d_coord_f = 0;
                                    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
                                        d_coord_f += ii[i] * d_a;
                                        d_a *= d_pshape[i];
                                    }
                                    int64_t s_coord_f = desp[7];
                                    int64_t s_a = d_shape[7];
                                    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
                                        s_coord_f += (desp[i] + ii[i]) * s_a;
                                        s_a *= d_shape[i];
                                    }
                                    memcpy(chunk + d_coord_f * typesize, bbuffer + s_coord_f * typesize, seq_copylen);
                                }
                            }
                        }
                    }
                }
            }
        }
        CATERVA_ERROR(caterva_array_append(array, chunk, (size_t) array->chunksize * typesize));
    }
    ctx->cfg->free(chunk);

    return CATERVA_SUCCEED;
}


int caterva_blosc_to_buffer(caterva_array_t *src, void *dest) {
    int8_t *d_b = (int8_t *) dest;
    int64_t s_shape[CATERVA_MAXDIM];
    int64_t s_pshape[CATERVA_MAXDIM];
    int64_t s_eshape[CATERVA_MAXDIM];
    int8_t s_ndim = src->ndim;

    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        s_shape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->shape[i];
        s_eshape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->extendedshape[i];
        s_pshape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->chunkshape[i];
    }

    /* Initialise a chunk buffer */
    caterva_context_t *ctx = src->ctx;
    int typesize = src->sc->typesize;
    int8_t *chunk = (int8_t *) ctx->alloc((size_t) src->chunksize * typesize);
    CATERVA_ERROR_NULL(chunk);
    /* Calculate the constants out of the for  */
    int64_t aux[CATERVA_MAXDIM];
    aux[7] = s_eshape[7] / s_pshape[7];
    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
        aux[i] = s_eshape[i] / s_pshape[i] * aux[i + 1];
    }

    /* Fill array from schunk (chunk by chunk) */
    int64_t desp[CATERVA_MAXDIM], r[CATERVA_MAXDIM];
    for (int64_t ci = 0; ci < src->extendedesize / src->chunksize; ci++) {
        if (blosc2_schunk_decompress_chunk(src->sc, (int) ci, chunk, (size_t) src->chunksize * typesize) < 0) {
            CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
        }
        /* Calculate the coord. of the chunk first element in arr buffer */
        desp[7] = ci % aux[7] * s_pshape[7];
        for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
            desp[i] = ci % (aux[i]) / (aux[i + 1]) * s_pshape[i];
        }
        /* Calculate if pad with 0 are needed in this chunk */
        for (int i = CATERVA_MAXDIM - 1; i >= 0; i--) {
            if (desp[i] + s_pshape[i] > s_shape[i]) {
                r[i] = s_shape[i] - desp[i];
            } else {
                r[i] = s_pshape[i];
            }
        }

        /* Copy each line of data from chunk to arr */
        int64_t s_coord_f, d_coord_f, s_a, d_a;
        int64_t ii[CATERVA_MAXDIM];
        for (ii[6] = 0; ii[6] < r[6]; ii[6]++) {
            for (ii[5] = 0; ii[5] < r[5]; ii[5]++) {
                for (ii[4] = 0; ii[4] < r[4]; ii[4]++) {
                    for (ii[3] = 0; ii[3] < r[3]; ii[3]++) {
                        for (ii[2] = 0; ii[2] < r[2]; ii[2]++) {
                            for (ii[1] = 0; ii[1] < r[1]; ii[1]++) {
                                for (ii[0] = 0; ii[0] < r[0]; ii[0]++) {
                                    s_coord_f = 0;
                                    s_a = s_pshape[7];
                                    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
                                        s_coord_f += ii[i] * s_a;
                                        s_a *= s_pshape[i];
                                    }
                                    d_coord_f = desp[7];
                                    d_a = s_shape[7];
                                    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
                                        d_coord_f += (desp[i] + ii[i]) * d_a;
                                        d_a *= s_shape[i];
                                    }
                                    memcpy(&d_b[d_coord_f * typesize], &chunk[s_coord_f * typesize],
                                           r[7] * typesize);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    ctx->free(chunk);
    return CATERVA_SUCCEED;
}


int caterva_blosc_get_slice_buffer(void *dest, caterva_array_t *src, caterva_dims_t *start,
                                   caterva_dims_t *stop, caterva_dims_t *d_pshape) {
    uint8_t *bdest = dest;   // for allowing pointer arithmetic
    int64_t start_[CATERVA_MAXDIM];
    int64_t stop_[CATERVA_MAXDIM];
    int64_t d_pshape_[CATERVA_MAXDIM];
    int64_t s_pshape[CATERVA_MAXDIM];
    int64_t s_eshape[CATERVA_MAXDIM];
    int8_t s_ndim = src->ndim;

    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        start_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = start->dims[i];
        stop_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = stop->dims[i];
        d_pshape_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = d_pshape->dims[i];
        s_eshape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->extendedshape[i];
        s_pshape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->chunkshape[i];
    }

    // Acceleration path for the case where we are doing (1-dim) aligned chunk reads
    if ((s_ndim == 1) && (src->chunkshape[0] == d_pshape->dims[0]) &&
        (start->dims[0] % src->chunkshape[0] == 0) && (stop->dims[0] % src->chunkshape[0] == 0)) {
        int nchunk = (int)(start->dims[0] / src->chunkshape[0]);
        // In case of an aligned read, decompress directly in destination
        if (blosc2_schunk_decompress_chunk(src->sc, nchunk, bdest, (size_t)src->chunksize * src->sc->typesize) < 0) {
            CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
        }
        return CATERVA_SUCCEED;
    }

    for (int j = 0; j < CATERVA_MAXDIM - s_ndim; ++j) {
        start_[j] = 0;
    }
    /* Create chunk buffers */
    caterva_context_t *ctx = src->ctx;
    int typesize = src->sc->typesize;

    uint8_t *chunk;
    bool local_cache;
    if (src->part_cache.data == NULL) {
        chunk = (uint8_t *) ctx->alloc((size_t) src->chunksize * typesize);
        CATERVA_ERROR_NULL(chunk);
        local_cache = true;
    } else {
        chunk = src->part_cache.data;
        local_cache = false;
    }
    int64_t i_start[8], i_stop[8];
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        i_start[i] = start_[i] / s_pshape[i];
        i_stop[i] = (stop_[i] - 1) / s_pshape[i];
    }

    /* Calculate the used chunks */
    int64_t ii[CATERVA_MAXDIM], jj[CATERVA_MAXDIM];
    int64_t c_start[CATERVA_MAXDIM], c_stop[CATERVA_MAXDIM];
    for (ii[0] = i_start[0]; ii[0] <= i_stop[0]; ++ii[0]) {
        for (ii[1] = i_start[1]; ii[1] <= i_stop[1]; ++ii[1]) {
            for (ii[2] = i_start[2]; ii[2] <= i_stop[2]; ++ii[2]) {
                for (ii[3] = i_start[3]; ii[3] <= i_stop[3]; ++ii[3]) {
                    for (ii[4] = i_start[4]; ii[4] <= i_stop[4]; ++ii[4]) {
                        for (ii[5] = i_start[5]; ii[5] <= i_stop[5]; ++ii[5]) {
                            for (ii[6] = i_start[6]; ii[6] <= i_stop[6]; ++ii[6]) {
                                for (ii[7] = i_start[7]; ii[7] <= i_stop[7]; ++ii[7]) {
                                    int nchunk = 0;
                                    int inc = 1;
                                    for (int i = CATERVA_MAXDIM - 1; i >= 0; --i) {
                                        nchunk += (int) (ii[i] * inc);
                                        inc *= (int) (s_eshape[i] / s_pshape[i]);
                                    }

                                    if ((src->part_cache.data == NULL) || (src->part_cache.nchunk != nchunk)) {
                                        if (blosc2_schunk_decompress_chunk(src->sc, nchunk, chunk,
                                                                       (size_t) src->chunksize * typesize) < 0) {
                                            CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
                                        };
                                    }
                                    if (src->part_cache.data != NULL) {
                                        src->part_cache.nchunk = nchunk;
                                    }

                                    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
                                        if (ii[i] == (start_[i] / s_pshape[i])) {
                                            c_start[i] = start_[i] % s_pshape[i];
                                        } else {
                                            c_start[i] = 0;
                                        }
                                        if (ii[i] == stop_[i] / s_pshape[i]) {
                                            c_stop[i] = stop_[i] % s_pshape[i];
                                        } else {
                                            c_stop[i] = s_pshape[i];
                                        }
                                    }
                                    jj[7] = c_start[7];
                                    for (jj[0] = c_start[0]; jj[0] < c_stop[0]; ++jj[0]) {
                                        for (jj[1] = c_start[1]; jj[1] < c_stop[1]; ++jj[1]) {
                                            for (jj[2] = c_start[2]; jj[2] < c_stop[2]; ++jj[2]) {
                                                for (jj[3] = c_start[3]; jj[3] < c_stop[3]; ++jj[3]) {
                                                    for (jj[4] = c_start[4]; jj[4] < c_stop[4]; ++jj[4]) {
                                                        for (jj[5] = c_start[5]; jj[5] < c_stop[5]; ++jj[5]) {
                                                            for (jj[6] = c_start[6]; jj[6] < c_stop[6]; ++jj[6]) {
                                                                int64_t chunk_pointer = 0;
                                                                int64_t chunk_pointer_inc = 1;
                                                                for (int i = CATERVA_MAXDIM - 1; i >= 0; --i) {
                                                                    chunk_pointer += jj[i] * chunk_pointer_inc;
                                                                    chunk_pointer_inc *= s_pshape[i];
                                                                }
                                                                int64_t buf_pointer = 0;
                                                                int64_t buf_pointer_inc = 1;
                                                                for (int i = CATERVA_MAXDIM - 1; i >= 0; --i) {
                                                                    buf_pointer += (jj[i] + s_pshape[i] * ii[i] -
                                                                        start_[i]) * buf_pointer_inc;
                                                                    buf_pointer_inc *= d_pshape_[i];
                                                                }
                                                                memcpy(&bdest[buf_pointer * typesize],
                                                                       &chunk[chunk_pointer * typesize],
                                                                       (c_stop[7] - c_start[7]) * typesize);
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (local_cache) {
        ctx->free(chunk);
    }

    return CATERVA_SUCCEED;
}


int caterva_blosc_get_slice(caterva_array_t *dest, caterva_array_t *src, caterva_dims_t *start,
                            caterva_dims_t *stop) {

    caterva_context_t *ctx = src->ctx;
    int typesize = ctx->cparams.typesize;

    uint8_t *chunk = ctx->alloc((size_t) dest->chunksize * typesize);
    CATERVA_ERROR_NULL(chunk);
    int64_t d_pshape[CATERVA_MAXDIM];
    int64_t d_start[CATERVA_MAXDIM];
    int64_t d_stop[CATERVA_MAXDIM];
    int8_t d_ndim = dest->ndim;
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        d_pshape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = dest->chunkshape[i];
        d_start[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = start->dims[i];
        d_stop[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = stop->dims[i];
    }
    int64_t ii[CATERVA_MAXDIM];
    for (ii[0] = d_start[0]; ii[0] < d_stop[0]; ii[0] += d_pshape[0]) {
        for (ii[1] = d_start[1]; ii[1] < d_stop[1]; ii[1] += d_pshape[1]) {
            for (ii[2] = d_start[2]; ii[2] < d_stop[2]; ii[2] += d_pshape[2]) {
                for (ii[3] = d_start[3]; ii[3] < d_stop[3]; ii[3] += d_pshape[3]) {
                    for (ii[4] = d_start[4]; ii[4] < d_stop[4]; ii[4] += d_pshape[4]) {
                        for (ii[5] = d_start[5]; ii[5] < d_stop[5]; ii[5] += d_pshape[5]) {
                            for (ii[6] = d_start[6]; ii[6] < d_stop[6]; ii[6] += d_pshape[6]) {
                                for (ii[7] = d_start[7]; ii[7] < d_stop[7]; ii[7] += d_pshape[7]) {
                                    memset(chunk, 0, dest->chunksize * typesize);
                                    int64_t jj[CATERVA_MAXDIM];
                                    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
                                        if (ii[i] + d_pshape[i] > d_stop[i]) {
                                            jj[i] = d_stop[i];
                                        } else {
                                            jj[i] = ii[i] + d_pshape[i];
                                        }
                                    }
                                    int64_t start_[CATERVA_MAXDIM];
                                    int64_t stop_[CATERVA_MAXDIM];
                                    int64_t d_pshape_[CATERVA_MAXDIM];
                                    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
                                        start_[i] = ii[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM];
                                        stop_[i] = jj[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM];
                                        d_pshape_[i] = d_pshape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM];
                                    }
                                    caterva_dims_t start__ = caterva_new_dims(start_, d_ndim);
                                    caterva_dims_t stop__ = caterva_new_dims(stop_, d_ndim);
                                    caterva_dims_t d_pshape__ = caterva_new_dims(d_pshape_, d_ndim);
                                    CATERVA_ERROR(
                                        caterva_array_get_slice_buffer(chunk, src, &start__, &stop__, &d_pshape__));
                                    CATERVA_ERROR(caterva_array_append(dest, chunk, dest->chunksize * typesize));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    free(chunk);

    return CATERVA_SUCCEED;
}

int caterva_blosc_squeeze(caterva_array_t *src) {
    uint8_t nones = 0;
    int64_t newshape_[CATERVA_MAXDIM];
    int32_t newpshape_[CATERVA_MAXDIM];

    for (int i = 0; i < src->ndim; ++i) {
        if (src->shape[i] != 1) {
            newshape_[nones] = src->shape[i];
            newpshape_[nones] = src->chunkshape[i];
            nones += 1;
        }
    }
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        if (i < nones) {
            src->chunkshape[i] = newpshape_[i];
        } else {
            src->chunkshape[i] = 1;
        }
    }

    src->ndim = nones;
    caterva_dims_t newshape = caterva_new_dims(newshape_, nones);
    CATERVA_ERROR(caterva_update_shape(src, &newshape));

    return CATERVA_SUCCEED;
}


int caterva_blosc_copy(caterva_array_t *dest, caterva_array_t *src) {
    int64_t start_[CATERVA_MAXDIM] = {0, 0, 0, 0, 0, 0, 0, 0};
    caterva_dims_t start = caterva_new_dims(start_, src->ndim);
    int64_t stop_[CATERVA_MAXDIM];
    for (int i = 0; i < src->ndim; ++i) {
        stop_[i] = src->shape[i];
    }
    caterva_dims_t stop = caterva_new_dims(stop_, src->ndim);

    CATERVA_ERROR(caterva_array_get_slice(dest, src, &start, &stop));

    return CATERVA_SUCCEED;
}


int caterva_blosc_update_shape(caterva_array_t *carr, caterva_dims_t *shape) {
    if (carr->ndim != shape->ndim) {
        CATERVA_ERROR(CATERVA_ERR_INVALID_ARGUMENT);
    }
    carr->size = 1;
    carr->extendedesize = 1;
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        carr->shape[i] = shape->dims[i];
        if (i < shape->ndim) {
            if (shape->dims[i] % carr->chunkshape[i] == 0) {
                carr->extendedshape[i] = shape->dims[i];
            } else {
                carr->extendedshape[i] = shape->dims[i] + carr->chunkshape[i] - shape->dims[i] % carr->chunkshape[i];
            }
        } else {
            carr->extendedshape[i] = 1;
        }
        carr->size *= carr->shape[i];
        carr->extendedesize *= carr->extendedshape[i];
    }

    uint8_t *smeta = NULL;
    // Serialize the dimension info ...
    int32_t smeta_len = serialize_meta(carr->ndim, carr->shape, carr->chunkshape, &smeta);
    if (smeta_len < 0) {
        fprintf(stderr, "error during serializing dims info for Caterva");
        return -1;
    }
    // ... and update it in its metalayer
    if (blosc2_has_metalayer(carr->sc, "caterva") < 0) {
        if (blosc2_add_metalayer(carr->sc, "caterva", smeta, (uint32_t) smeta_len) < 0) {
            CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
        }
    }
    else {
        if (blosc2_update_metalayer(carr->sc, "caterva", smeta, (uint32_t) smeta_len) < 0) {
            CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
        }
    }

    return CATERVA_SUCCEED;
}


int caterva_blosc_array_empty(caterva_context_t *ctx, caterva_params_t *params, caterva_storage_t *storage,
                              caterva_array_t **array) {
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
    int32_t *chunkshape = storage->properties.blosc.chunkshape;
    (*array)->size = 1;
    (*array)->chunksize = 1;
    (*array)->extendedesize = 1;

    for (int i = 0; i < params->ndim; ++i) {
        (*array)->shape[i] = shape[i];
        (*array)->chunkshape[i] = chunkshape[i];
        if (shape[i] % chunkshape[i] == 0) {
            (*array)->extendedshape[i] = shape[i];
        } else {
            (*array)->extendedshape[i] = shape[i] + chunkshape[i] - shape[i] % chunkshape[i];
        }
        (*array)->size *= shape[i];
        (*array)->chunksize *= chunkshape[i];
        (*array)->extendedesize *= (*array)->extendedshape[i];
    }

    for (int i = params->ndim; i < CATERVA_MAXDIM; ++i) {
        (*array)->shape[i] = 1;
        (*array)->chunkshape[i] = 1;
        (*array)->extendedshape[i] = 1;
    }

    // The partition cache (empty initially)
    (*array)->part_cache.data = NULL;
    (*array)->part_cache.nchunk = -1;  // means no valid cache yet

    (*array)->buf = NULL;

    blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
    cparams.blocksize = 0; //TODO: Update when the blockshape is added
    cparams.schunk = NULL;
    cparams.typesize = params->itemsize;
    cparams.prefilter = ctx->cfg->prefilter;
    cparams.pparams = ctx->cfg->pparams;
    cparams.use_dict = ctx->cfg->usedict;
    cparams.nthreads = ctx->cfg->nthreads;
    cparams.clevel = ctx->cfg->complevel;
    cparams.compcode = ctx->cfg->compcodec;
    for (int i = 0; i < BLOSC2_MAX_FILTERS; ++i) {
        cparams.filters[i] = ctx->cfg->filters[i];
        cparams.filters_meta[i] = ctx->cfg->filtersmeta[i];
    }

    blosc2_dparams dparams = BLOSC2_DPARAMS_DEFAULTS;
    dparams.schunk = NULL;
    dparams.nthreads = ctx->cfg->nthreads;

    blosc2_frame *frame = NULL;
    if (storage->properties.blosc.enforceframe) {
        char *fname = NULL;
        if (storage->properties.blosc.filename) {
            fname = storage->properties.blosc.filename;
        }
        frame = blosc2_new_frame(fname);
    }

    blosc2_schunk *sc = blosc2_new_schunk(cparams, dparams, frame);
    if (sc == NULL) {
        DEBUG_PRINT("Pointer is NULL");
        return CATERVA_ERR_BLOSC_FAILED;
    }

    if (frame != NULL) {
        // Serialize the dimension info in the associated frame
        if (sc->nmetalayers >= BLOSC2_MAX_METALAYERS) {
            DEBUG_PRINT("the number of metalayers for this frame has been exceeded");
            return CATERVA_ERR_BLOSC_FAILED;
        }
        uint8_t *smeta = NULL;
        int32_t smeta_len = serialize_meta(params->ndim, shape, chunkshape, &smeta);
        if (smeta_len < 0) {
            DEBUG_PRINT("error during serializing dims info for Caterva");
            return CATERVA_ERR_BLOSC_FAILED;
        }
        // And store it in caterva metalayer
        if (blosc2_add_metalayer(sc, "caterva", smeta, (uint32_t)smeta_len) < 0) {
            return CATERVA_ERR_BLOSC_FAILED;
        }
        free(smeta);
    }

    (*array)->sc = sc;

    return CATERVA_SUCCEED;
}
