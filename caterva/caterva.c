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
#include "caterva.h"
#include "assert.h"

caterva_ctx_t *caterva_new_ctx(void *(*c_alloc)(size_t), void (*c_free)(void *), blosc2_cparams cparams, blosc2_dparams dparams) {
    caterva_ctx_t *ctx;
    if (c_alloc == NULL) {
        ctx = (caterva_ctx_t *) malloc(sizeof(caterva_ctx_t));
        ctx->alloc = malloc;
    } else {
        ctx = (caterva_ctx_t *) c_alloc(sizeof(caterva_ctx_t));
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

caterva_dims_t caterva_new_dims(int64_t *dims, int8_t ndim) {
    caterva_dims_t dims_s = CATERVA_DIMS_DEFAULTS;
    for (int i = 0; i < ndim; ++i) {
        dims_s.dims[i] = dims[i];
    }
    dims_s.ndim = ndim;
    return dims_s;
}

// Serialize the partition params
// TODO: use big-endian to encode ints
static int32_t serialize_meta(int8_t ndim, int64_t *shape, const int64_t *pshape, uint8_t **smeta) {
    int32_t max_smeta_len = 116;  // 4 + MAX_DIM * (1 + sizeof(int64_t)) + MAX_DIM * (1 + sizeof(int32_t))
    *smeta = malloc((size_t)max_smeta_len);
    uint8_t *pmeta = *smeta;

    // Build an array with 3 entries (ndim, shape, pshape)
    *pmeta++ = 0x90 + 3;

    // ndim entry
    *pmeta++ = (uint8_t)ndim;  // positive fixnum (7-bit positive integer)
    assert(pmeta - *smeta < max_smeta_len);

    // shape entry
    *pmeta++ = (uint8_t)(0x90) + ndim;  // fix array with ndim elements
    for (int8_t i = 0; i < ndim; i++) {
        *pmeta++ = 0xcf;  // uint64
        memcpy(pmeta, &(shape[i]), sizeof(int64_t));
        pmeta += sizeof(int64_t);
    }
    assert(pmeta - *smeta < max_smeta_len);

    // pshape entry
    *pmeta++ = (uint8_t)(0x90) + ndim;  // fix array with ndim elements
    for (int8_t i = 0; i < ndim; i++) {
        *pmeta++ = 0xd2;  // int32
        int32_t pshape_i = (int32_t)pshape[i];
        memcpy(pmeta, &pshape_i, sizeof(int32_t));
        pmeta += sizeof(int32_t);
    }
    assert(pmeta - *smeta <= max_smeta_len);

    int32_t slen = (int32_t)(pmeta - *smeta);
    *smeta = realloc(*smeta, (size_t)slen);  // get rid of the excess of bytes allocated

    return slen;
}

// Serialize the partition params
// TODO: decode big-endian ints to native endian
static int32_t deserialize_meta(uint8_t *smeta, uint32_t smeta_len, caterva_dims_t *shape, caterva_dims_t *pshape) {
    uint8_t *pmeta = smeta;

    // Check that we have an array with 3 entries (ndim, shape, pshape)
    assert(*pmeta == 0x90 + 3);
    pmeta += 1;
    assert(pmeta - smeta < smeta_len);

    // ndim entry
    int8_t ndim = pmeta[0];  // positive fixnum (7-bit positive integer)
    assert (ndim < CATERVA_MAXDIM);
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
        assert(*pmeta == 0xcf);   // uint64
        pmeta += 1;
        memcpy(&(shape->dims[i]), pmeta, sizeof(int64_t));
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
        int32_t pshape_i = (int32_t)pshape->dims[i];
        memcpy(&pshape_i, pmeta, sizeof(int32_t));
        pshape->dims[i] = (int64_t)pshape_i;
        pmeta += sizeof(int32_t);
    }
    assert(pmeta - smeta <= smeta_len);

    uint32_t slen = (uint32_t)(pmeta - smeta);
    assert(slen == smeta_len);

    return 0;
}

caterva_array_t *caterva_empty_array(caterva_ctx_t *ctx, blosc2_frame *frame, caterva_dims_t *pshape) {
    /* Create a caterva_array_t buffer */
    caterva_array_t *carr = (caterva_array_t *) ctx->alloc(sizeof(caterva_array_t));
    carr->size = 1;
    carr->psize = 1;
    carr->esize = 1;
    // The partition cache (empty initially)
    carr->part_cache.data = NULL;
    carr->part_cache.nchunk = -1;  // means no valid cache yet
    carr->sc = NULL;
    carr->buf = NULL;

    if (pshape != NULL) {
        carr->storage = CATERVA_STORAGE_BLOSC;
        carr->ndim = pshape->ndim;
        for (unsigned int i = 0; i < CATERVA_MAXDIM; i++) {
            carr->pshape[i] = pshape->dims[i];
            carr->shape[i] = 1;
            carr->eshape[i] = 1;
            carr->psize *= carr->pshape[i];
        }

        if (frame != NULL) {
            // Serialize the dimension info in the associated frame
            if (frame->nmetalayers >= BLOSC2_MAX_METALAYERS) {
                fprintf(stderr, "the number of metalayers for this frame has been exceeded\n");
                return NULL;
            }
            uint8_t *smeta = NULL;
            int32_t smeta_len = serialize_meta(carr->ndim, carr->shape, carr->pshape, &smeta);
            if (smeta_len < 0) {
                fprintf(stderr, "error during serializing dims info for Caterva");
                return NULL;
            }
            // And store it in caterva metalayer
            int retcode = blosc2_frame_add_metalayer(frame, "caterva", smeta, (uint32_t)smeta_len);
            if (retcode < 0) {
                return NULL;
            }
            free(smeta);
        }

        /* Create a schunk (for a frame-disk-backed one, this implies serializing the header on-disk */
        blosc2_schunk *sc = blosc2_new_schunk(ctx->cparams, ctx->dparams, frame);
        carr->sc = sc;
    } else {
        carr->storage = CATERVA_STORAGE_PLAINBUFFER;
    }

    /* Copy context to caterva_array_t */
    carr->ctx = (caterva_ctx_t *) ctx->alloc(sizeof(caterva_ctx_t));
    memcpy(&carr->ctx[0], &ctx[0], sizeof(caterva_ctx_t));

    return carr;
}

caterva_array_t *caterva_from_file(caterva_ctx_t *ctx, const char *filename) {
    /* Create a caterva_array_t buffer */
    caterva_array_t *carr = (caterva_array_t *) ctx->alloc(sizeof(caterva_array_t));

    /* Copy context to caterva_array_t */
    carr->ctx = (caterva_ctx_t *) ctx->alloc(sizeof(caterva_ctx_t));
    memcpy(&carr->ctx[0], &ctx[0], sizeof(caterva_ctx_t));

    // Open the frame on-disk...
    blosc2_frame *frame = blosc2_frame_from_file(filename);
    /* ...and create a schunk out of it */
    blosc2_schunk *sc = blosc2_schunk_from_frame(frame, false);  // do not create an sparse chunk
    carr->sc = sc;
    carr->storage = CATERVA_STORAGE_BLOSC;

    blosc2_dparams *dparams;
    blosc2_get_dparams(carr->sc, &dparams);
    blosc2_cparams *cparams;
    blosc2_get_cparams(carr->sc, &cparams);

    memcpy(&carr->ctx->dparams, dparams, sizeof(blosc2_dparams));
    memcpy(&carr->ctx->cparams, cparams, sizeof(blosc2_cparams));

    // Deserialize the caterva metalayer
    caterva_dims_t shape;
    caterva_dims_t pshape;
    uint8_t *smeta;
    uint32_t smeta_len;
    blosc2_frame_get_metalayer(frame, "caterva", &smeta, &smeta_len);
    deserialize_meta(smeta, smeta_len, &shape, &pshape);
    carr->size = 1;
    carr->psize = 1;
    carr->esize = 1;
    carr->ndim = pshape.ndim;

    for (int i = 0; i < CATERVA_MAXDIM; i++) {
        carr->shape[i] = shape.dims[i];
        carr->size *= shape.dims[i];
        carr->pshape[i] = pshape.dims[i];
        carr->psize *= pshape.dims[i];
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

    return carr;
}

int caterva_free_ctx(caterva_ctx_t *ctx) {
    ctx->free(ctx);
    return 0;
}

int caterva_free_array(caterva_array_t *carr) {
    switch (carr->storage) {
        case CATERVA_STORAGE_BLOSC:
            blosc2_free_schunk(carr->sc);
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            if (carr->buf != NULL) {
                carr->ctx->free(carr->buf);
            }
    }

    void (*aux_free)(void *) = carr->ctx->free;
    caterva_free_ctx(carr->ctx);
    aux_free(carr);
    return 0;
}


int _caterva_update_shape(caterva_array_t *carr, caterva_dims_t *shape) {
    if (carr->storage == CATERVA_STORAGE_BLOSC) {
        if (carr->ndim != shape->ndim) {
            printf("caterva array ndim and shape ndim are not equal\n");
            return -1;
        }
        carr->size = 1;
        carr->esize = 1;
        for (int i = 0; i < CATERVA_MAXDIM; ++i) {
            carr->shape[i] = shape->dims[i];

            if (i < shape->ndim) {
                if (shape->dims[i] % carr->pshape[i] == 0) {
                    carr->eshape[i] = shape->dims[i];
                } else {
                    carr->eshape[i] = shape->dims[i] + carr->pshape[i] - shape->dims[i] % carr->pshape[i];
                }
            } else {
                carr->eshape[i] = 1;
            }
            carr->size *= carr->shape[i];
            carr->esize *= carr->eshape[i];
        }

        blosc2_frame *frame = carr->sc->frame;
        if (frame != NULL) {
            uint8_t *smeta = NULL;
            // Serialize the dimension info ...
            int32_t smeta_len = serialize_meta(carr->ndim, carr->shape, carr->pshape, &smeta);
            if (smeta_len < 0) {
                fprintf(stderr, "error during serializing dims info for Caterva");
                return -1;
            }
            // ... and update it in its metalayer
            int retcode = blosc2_frame_update_metalayer(frame, "caterva", smeta, (uint32_t) smeta_len);
            if (retcode < 0) {
                return -1;
            }
        }
    } else {
        carr->ndim = shape->ndim;
        carr->size = 1;
        carr->esize = 1;
        for (int i = 0; i < CATERVA_MAXDIM; ++i) {
            carr->shape[i] = shape->dims[i];
            carr->eshape[i] = shape->dims[i];
            carr->pshape[i] = shape->dims[i];
            carr->size *= carr->shape[i];
            carr->esize *= carr->eshape[i];
            carr->psize *= carr->pshape[i];
        }
    }

    return 0;
}

// Fill a caterva array from a C buffer
// The caterva array must be empty at the begining
int caterva_from_buffer(caterva_array_t *dest, caterva_dims_t *shape, void *src) {

    int8_t *s_b = (int8_t *) src;

    _caterva_update_shape(dest, shape);

    if (dest->storage == CATERVA_STORAGE_BLOSC) {

        if (dest->sc->nbytes > 0) {
            printf("Caterva container must be empty!");
            return -1;
        }
        int64_t d_shape[CATERVA_MAXDIM];
        int64_t d_pshape[CATERVA_MAXDIM];
        int64_t d_eshape[CATERVA_MAXDIM];
        int8_t d_ndim = dest->ndim;

        for (int i = 0; i < CATERVA_MAXDIM; ++i) {
            d_shape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = dest->shape[i];
            d_eshape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = dest->eshape[i];
            d_pshape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = dest->pshape[i];
        }

        caterva_ctx_t *ctx = dest->ctx;
        int32_t typesize = dest->sc->typesize;
        int8_t *chunk = malloc((size_t) dest->psize * typesize);

        /* Calculate the constants out of the for  */
        int64_t aux[CATERVA_MAXDIM];
        aux[7] = d_eshape[7] / d_pshape[7];
        for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
            aux[i] = d_eshape[i] / d_pshape[i] * aux[i + 1];
        }

        /* Fill each chunk buffer */
        int64_t desp[CATERVA_MAXDIM], r[CATERVA_MAXDIM];
        for (int64_t ci = 0; ci < dest->esize / dest->psize; ci++) {
            memset(chunk, 0, dest->psize * typesize);

            /* Calculate the coord. of the chunk first element */
            desp[7] = ci % (d_eshape[7] / d_pshape[7]) * d_pshape[7];
            for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
                desp[i] = ci % (aux[i]) / (aux[i + 1]) * d_pshape[i];
            }

            /* Calculate if pad with 0 are needed in this chunk */
            for (int i = CATERVA_MAXDIM - 1; i >= 0; i--) {
                if (desp[i] + d_pshape[i] > d_shape[i]) {
                    r[i] = d_shape[i] - desp[i];
                } else {
                    r[i] = d_pshape[i];
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
                                        d_coord_f = 0;
                                        d_a = d_pshape[7];

                                        for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
                                            d_coord_f += ii[i] * d_a;
                                            d_a *= d_pshape[i];
                                        }

                                        s_coord_f = desp[7];
                                        s_a = d_shape[7];

                                        for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
                                            s_coord_f += (desp[i] + ii[i]) * s_a;
                                            s_a *= d_shape[i];
                                        }
                                        memcpy(&chunk[d_coord_f * typesize], &s_b[s_coord_f * typesize],
                                               r[7] * typesize);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            blosc2_schunk_append_buffer(dest->sc, chunk, (size_t) dest->psize * typesize);
        }

        ctx->free(chunk);
    } else { // Plain buffer

        if (dest->buf != NULL) {
            printf("Caterva container must be empty!");
            return -1;
        }
        dest->buf = malloc(dest->size * (size_t) dest->ctx->cparams.typesize);
        memcpy(dest->buf, src, dest->size * (size_t) dest->ctx->cparams.typesize);
    }

    return 0;
}

int caterva_fill(caterva_array_t *dest, caterva_dims_t *shape, void *value) {

    _caterva_update_shape(dest, shape);

    if (dest->storage == CATERVA_STORAGE_BLOSC) {
        uint8_t *chunk = malloc((size_t) dest->psize * dest->sc->typesize);

        for (int64_t i = 0; i < dest->psize; ++i) {
            if (dest->sc->typesize == 1) {
                chunk[i] = *(uint8_t *) value;
            } else if (dest->sc->typesize == 2) {
                ((uint16_t *) chunk)[i] = *(uint16_t *) value;
            } else if (dest->sc->typesize == 4) {
                ((uint32_t *) chunk)[i] = *(uint32_t *) value;
            } else {
                ((int64_t *) chunk)[i] = *(int64_t *) value;
            }
        }

        int64_t nchunk = dest->esize / dest->psize;

        for (int i = 0; i < nchunk; ++i) {
            blosc2_schunk_append_buffer(dest->sc, chunk, (size_t) dest->psize * dest->sc->typesize);
        }

        free(chunk);
    } else {
        dest->buf = malloc(dest->size * (size_t) dest->ctx->cparams.typesize);
        for (int i = 0; i < dest->size; ++i) {
            switch (dest->ctx->cparams.typesize) {
                case 8:
                    ((uint64_t *) dest->buf)[i] = *(uint64_t *) value;
                    break;
                case 4:
                    ((uint32_t *) dest->buf)[i] = *(uint32_t *) value;
                    break;
                case 2:
                    ((uint16_t *) dest->buf)[i] = *(uint16_t *) value;
                    break;
                case 1:
                    ((uint8_t *) dest->buf)[i] = *(uint8_t *) value;
                    break;
                default:
                    break;
            }
        }
    }
    return 0;
}

int caterva_to_buffer(caterva_array_t *src, void *dest) {

    if (src->storage == CATERVA_STORAGE_BLOSC) {
        int8_t *d_b = (int8_t *) dest;

        int64_t s_shape[CATERVA_MAXDIM];
        int64_t s_pshape[CATERVA_MAXDIM];
        int64_t s_eshape[CATERVA_MAXDIM];
        int8_t s_ndim = src->ndim;

        for (int i = 0; i < CATERVA_MAXDIM; ++i) {
            s_shape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->shape[i];
            s_eshape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->eshape[i];
            s_pshape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->pshape[i];
        }

        /* Initialise a chunk buffer */
        caterva_ctx_t *ctx = src->ctx;
        int typesize = src->sc->typesize;
        int8_t *chunk = (int8_t *) ctx->alloc((size_t) src->psize * typesize);

        /* Calculate the constants out of the for  */
        int64_t aux[CATERVA_MAXDIM];
        aux[7] = s_eshape[7] / s_pshape[7];
        for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
            aux[i] = s_eshape[i] / s_pshape[i] * aux[i + 1];
        }

        /* Fill array from schunk (chunk by chunk) */
        int64_t desp[CATERVA_MAXDIM], r[CATERVA_MAXDIM];
        for (int64_t ci = 0; ci < src->esize / src->psize; ci++) {
            blosc2_schunk_decompress_chunk(src->sc, (int) ci, chunk, (size_t) src->psize * typesize);

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
    } else {
        memcpy(dest, src->buf, src->size * (size_t) src->ctx->cparams.typesize);
    }
    return 0;
}

int caterva_get_slice_buffer(void *dest, caterva_array_t *src, caterva_dims_t *start,
                             caterva_dims_t *stop, caterva_dims_t *d_pshape) {
	uint8_t *bdest = dest;   // for allowing pointer arithmetic
    int64_t start_[CATERVA_MAXDIM];
    int64_t stop_[CATERVA_MAXDIM];
    int64_t d_pshape_[CATERVA_MAXDIM];
    int64_t s_pshape[CATERVA_MAXDIM];
    int64_t s_eshape[CATERVA_MAXDIM];
    int8_t s_ndim = src->ndim;

    if (src->storage == CATERVA_STORAGE_BLOSC) {
        for (int i = 0; i < CATERVA_MAXDIM; ++i) {
            start_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = start->dims[i];
            stop_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = stop->dims[i];
            d_pshape_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = d_pshape->dims[i];
            s_eshape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->eshape[i];
            s_pshape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->pshape[i];
        }

        // Acceleration path for the case where we are doing (1-dim) aligned chunk reads
        if ((s_ndim == 1) && (src->pshape[0] == d_pshape->dims[0]) &&
            (start->dims[0] % src->pshape[0] == 0) && (stop->dims[0] % src->pshape[0] == 0)) {
            int nchunk = (int)(start->dims[0] / src->pshape[0]);
            // In case of an aligned read, decompress directly in destination
            blosc2_schunk_decompress_chunk(src->sc, nchunk, bdest, (size_t)src->psize * src->sc->typesize);
            return 0;
        }


        for (int j = 0; j < CATERVA_MAXDIM - s_ndim; ++j) {
            start_[j] = 0;
        }
         /* Create chunk buffers */
        caterva_ctx_t *ctx = src->ctx;
        int typesize = src->sc->typesize;

        uint8_t *chunk;
        bool local_cache;
        if (src->part_cache.data == NULL) {
            chunk = (uint8_t *) ctx->alloc((size_t) src->psize * typesize);
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
                                            blosc2_schunk_decompress_chunk(src->sc, nchunk, chunk,
                                                                           (size_t) src->psize * typesize);
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
    } else {
        caterva_dims_t shape = caterva_get_shape(src);
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
            d_pshape_[(CATERVA_MAXDIM - s_ndim + j) % CATERVA_MAXDIM] = (stop_[j] - start_[j]);

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

    }
    return 0;
}

int caterva_get_slice_buffer_no_copy(void **dest, caterva_array_t *src, caterva_dims_t *start,
                                     caterva_dims_t *stop, caterva_dims_t *d_pshape) {

    int64_t start_[CATERVA_MAXDIM];
    int64_t stop_[CATERVA_MAXDIM];
    int64_t d_pshape_[CATERVA_MAXDIM];
    int8_t s_ndim = src->ndim;

    caterva_dims_t shape = caterva_get_shape(src);
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
        d_pshape_[(CATERVA_MAXDIM - s_ndim + j) % CATERVA_MAXDIM] = (stop_[j] - start_[j]);

    }

    int64_t chunk_pointer = 0;
    int64_t chunk_pointer_inc = 1;
    for (int i = CATERVA_MAXDIM - 1; i >= 0; --i) {
        chunk_pointer += start_[i] * chunk_pointer_inc;
        chunk_pointer_inc *= s_shape[i];
    }
    *dest = &src->buf[chunk_pointer * src->ctx->cparams.typesize];
    //printf("CA %p - %p\n", *dest, (void *) &src->buf[chunk_pointer * src->ctx->cparams.typesize]);
    return 0;
}

int caterva_set_slice_buffer(caterva_array_t *dest, void *src, caterva_dims_t *start,
                             caterva_dims_t *stop) {
    if (dest->storage == CATERVA_STORAGE_BLOSC) {
        return -1;
    }

    uint8_t *bsrc = src;   // for allowing pointer arithmetic
    int64_t start_[CATERVA_MAXDIM];
    int64_t stop_[CATERVA_MAXDIM];
    int8_t s_ndim = dest->ndim;

    if (dest->storage == CATERVA_STORAGE_PLAINBUFFER) {
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
    }

    return 0;
}

int caterva_get_slice(caterva_array_t *dest, caterva_array_t *src, caterva_dims_t *start, caterva_dims_t *stop){

    if (start->ndim != stop->ndim) {
        return -1;
    }
    if (start->ndim != src->ndim) {
        return -1;
    }
    if (src->storage != dest->storage) {
        return -1;
    }

    caterva_ctx_t *ctx = src->ctx;
    int typesize = ctx->cparams.typesize;

    int64_t shape_[CATERVA_MAXDIM];
    for (int i = 0; i < start->ndim; ++i) {
        shape_[i] = stop->dims[i] - start->dims[i];
    }
    for (int i = (int) start->ndim; i < CATERVA_MAXDIM; ++i) {
        shape_[i] = 1;
        start->dims[i] = 0;
    }

    caterva_dims_t shape = caterva_new_dims(shape_, start->ndim);
    _caterva_update_shape(dest, &shape);

    if (src->storage == CATERVA_STORAGE_BLOSC) {

        uint8_t *chunk = ctx->alloc((size_t) dest->psize * typesize);

        int64_t d_pshape[CATERVA_MAXDIM];
        int64_t d_start[CATERVA_MAXDIM];
        int64_t d_stop[CATERVA_MAXDIM];
        int8_t d_ndim = dest->ndim;

        for (int i = 0; i < CATERVA_MAXDIM; ++i) {
            d_pshape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = dest->pshape[i];
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
                                        memset(chunk, 0, dest->psize * typesize);
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

                                        caterva_get_slice_buffer(chunk, src, &start__, &stop__, &d_pshape__);

                                        blosc2_schunk_append_buffer(dest->sc, chunk, (size_t) dest->psize * typesize);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        free(chunk);
    } else {
        uint64_t size = 1;
        for (int i = 0; i < stop->ndim; ++i) {
            size *= stop->dims[i] - start->dims[i];
        }
        dest->buf = malloc(size * typesize);
        caterva_get_slice_buffer(dest->buf, src, start, stop, NULL);
    }
    return 0;
}

int caterva_repart(caterva_array_t *dest, caterva_array_t *src) {
    if (src->storage == CATERVA_STORAGE_BLOSC) {
        int64_t start_[CATERVA_MAXDIM] = {0, 0, 0, 0, 0, 0, 0, 0};
        caterva_dims_t start = caterva_new_dims(start_, dest->ndim);
        int64_t stop_[CATERVA_MAXDIM];
        for (int i = 0; i < dest->ndim; ++i) {
            stop_[i] = src->shape[i];
        }
        caterva_dims_t stop = caterva_new_dims(stop_, dest->ndim);
        caterva_get_slice(dest, src, &start, &stop);
        return 0;
    } else {
        return -1;
    }
}

int caterva_squeeze(caterva_array_t *src) {
    uint8_t nones = 0;
    int64_t newshape_[CATERVA_MAXDIM];
    int64_t newpshape_[CATERVA_MAXDIM];

    if (src->storage == CATERVA_STORAGE_BLOSC) {

        for (int i = 0; i < src->ndim; ++i) {
            if (src->shape[i] != 1) {
                newshape_[nones] = src->shape[i];
                newpshape_[nones] = src->pshape[i];
                nones += 1;
            }
        }

        for (int i = 0; i < CATERVA_MAXDIM; ++i) {
            if (i < nones) {
                src->pshape[i] = newpshape_[i];
            } else {
                src->pshape[i] = 1;
            }
        }
    } else {
        for (int i = 0; i < src->ndim; ++i) {
            if (src->shape[i] != 1) {
                newshape_[nones] = src->shape[i];
                nones += 1;
            }
        }
    }
    src->ndim = nones;

    caterva_dims_t newshape = caterva_new_dims(newshape_, nones);

    _caterva_update_shape(src, &newshape);

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
