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


static int32_t deserialize_meta(uint8_t *smeta, uint32_t smeta_len, caterva_dims_t *shape, caterva_dims_t *pshape) {
    uint8_t *pmeta = smeta;

    // Check that we have an array with 5 entries (version, ndim, shape, pshape, bshape)
    assert(*pmeta == 0x90 + 5);
    pmeta += 1;
    assert(pmeta - smeta < smeta_len);

    // version entry
    int8_t version = pmeta[0];  // positive fixnum (7-bit positive integer)
    assert (version <= CATERVA_METALAYER_VERSION);
    pmeta += 1;
    assert(pmeta - smeta < smeta_len);

    // ndim entry
    int8_t ndim = pmeta[0];  // positive fixnum (7-bit positive integer)
    assert (ndim <= CATERVA_MAXDIM);
    pmeta += 1;
    assert(pmeta - smeta < smeta_len);
    shape->ndim = ndim;
    pshape->ndim = ndim;

    // shape entry
    // Initialize to ones, as required by Caterva
    for (int i = 0; i < CATERVA_MAXDIM; i++) shape->dims[i] = 1;
    assert(*pmeta == (uint8_t)(0x90) + ndim);  // fix array with ndim elements
    pmeta += 1;
    for (int8_t i = 0; i < ndim; i++) {
        assert(*pmeta == 0xd3);   // int64
        pmeta += 1;
        swap_store(shape->dims + i, pmeta, sizeof(int64_t));
        pmeta += sizeof(int64_t);
    }
    assert(pmeta - smeta < smeta_len);

    // pshape entry
    // Initialize to ones, as required by Caterva
    for (int i = 0; i < CATERVA_MAXDIM; i++) pshape->dims[i] = 1;
    assert(*pmeta == (uint8_t)(0x90) + ndim);  // fix array with ndim elements
    pmeta += 1;
    for (int8_t i = 0; i < ndim; i++) {
        assert(*pmeta == 0xd2);  // int32
        pmeta += 1;
        swap_store(pshape->dims + i, pmeta, sizeof(int32_t));
        pmeta += sizeof(int32_t);
    }
    assert(pmeta - smeta <= smeta_len);

    // bshape entry
    // Initialize to ones, as required by Caterva
    // for (int i = 0; i < CATERVA_MAXDIM; i++) bshape->dims[i] = 1;
    assert(*pmeta == (uint8_t)(0x90) + ndim);  // fix array with ndim elements
    pmeta += 1;
    for (int8_t i = 0; i < ndim; i++) {
        assert(*pmeta == 0xd2);  // int32
        pmeta += 1;
        // swap_store(bshape->dims + i, pmeta, sizeof(int32_t));
        pmeta += sizeof(int32_t);
    }
    assert(pmeta - smeta <= smeta_len);
    uint32_t slen = (uint32_t)(pmeta - smeta);
    assert(slen == smeta_len);
    return 0;
}


caterva_array_t *caterva_blosc_from_frame(caterva_ctx_t *ctx, blosc2_frame *frame, bool copy) {
    /* Create a caterva_array_t buffer */
    caterva_array_t *carr = (caterva_array_t *) ctx->alloc(sizeof(caterva_array_t));

    /* Copy context to caterva_array_t */
    carr->ctx = (caterva_ctx_t *) ctx->alloc(sizeof(caterva_ctx_t));
    memcpy(&carr->ctx[0], &ctx[0], sizeof(caterva_ctx_t));

    /* Create a schunk out of the frame */
    blosc2_schunk *sc = blosc2_schunk_from_frame(frame, copy);
    carr->sc = sc;
    carr->storage = CATERVA_STORAGE_BLOSC;

    blosc2_dparams *dparams;
    blosc2_schunk_get_dparams(carr->sc, &dparams);
    blosc2_cparams *cparams;
    blosc2_schunk_get_cparams(carr->sc, &cparams);
    memcpy(&carr->ctx->dparams, dparams, sizeof(blosc2_dparams));
    memcpy(&carr->ctx->cparams, cparams, sizeof(blosc2_cparams));

    // Deserialize the caterva metalayer
    caterva_dims_t shape;
    caterva_dims_t pshape;
    uint8_t *smeta;
    uint32_t smeta_len;
    blosc2_get_metalayer(sc, "caterva", &smeta, &smeta_len);
    deserialize_meta(smeta, smeta_len, &shape, &pshape);
    carr->size = 1;
    carr->psize = 1;
    carr->esize = 1;
    carr->ndim = pshape.ndim;

    for (int i = 0; i < CATERVA_MAXDIM; i++) {
        carr->shape[i] = shape.dims[i];
        carr->size *= shape.dims[i];
        carr->pshape[i] = (int32_t)(pshape.dims[i]);
        carr->psize *= carr->pshape[i];
        if (shape.dims[i] % pshape.dims[i] == 0) {
            // The case for shape.dims[i] == 1 and pshape.dims[i] == 1 is handled here
            carr->eshape[i] = shape.dims[i];
        } else {
            carr->eshape[i] = shape.dims[i] + pshape.dims[i] - shape.dims[i] % pshape.dims[i];
        }
        carr->esize *= carr->eshape[i];
    }

    // The partition cache (empty initially)
    carr->part_cache.data = NULL;
    carr->part_cache.nchunk = -1;  // means no valid cache yet
    carr->empty = false;
    if (carr->sc->nchunks == carr->esize / carr->psize) {
        carr->filled = true;
    } else {
        carr->filled = false;
    }

    return carr;
}


caterva_array_t *caterva_blosc_from_sframe(caterva_ctx_t *ctx, uint8_t *sframe, int64_t len, bool copy) {
    // Generate a real frame first
    blosc2_frame *frame = blosc2_frame_from_sframe(sframe, len, copy);
    // ...and create a caterva array out of it
    caterva_array_t *array = caterva_from_frame(ctx, frame, copy);
    if (copy) {
        // We don't need the frame anymore
        blosc2_free_frame(frame);
    }
    return array;
}


caterva_array_t *caterva_blosc_from_file(caterva_ctx_t *ctx, const char *filename, bool copy) {
    // Open the frame on-disk...
    blosc2_frame *frame = blosc2_frame_from_file(filename);
    // ...and create a caterva array out of it
    caterva_array_t *array = caterva_from_frame(ctx, frame, copy);
    if (copy) {
        // We don't need the frame anymore
        blosc2_free_frame(frame);
    }
    return array;
}


int caterva_blosc_free_array(caterva_array_t *carr) {
    if (carr->sc != NULL) {
        blosc2_free_schunk(carr->sc);
    }
    void (*aux_free)(void *) = carr->ctx->free;
    caterva_free_ctx(carr->ctx);
    aux_free(carr);
    return 0;
}


int caterva_blosc_append(caterva_array_t *carr, void *part, int64_t partsize) {
    blosc2_schunk_append_buffer(carr->sc, part, partsize);

    carr->nparts++;
    if (carr->nparts == carr->esize / carr->psize) {
        carr->filled = true;
    }

    return 0;
}
