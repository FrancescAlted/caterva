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
#include "assert.h"
#include "caterva_blosc.h"
#include "caterva_plainbuffer.h"


#define CATERVA_UNUSED_PARAM(x) ((void)(x))


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
        pa2_[0] = pa_[1];
        break;
      default:
        fprintf(stderr, "Unhandled size: %d\n", size);
    }
  }
  memcpy(dest, pa2_, size);
  free(pa2_);
}


caterva_ctx_t *caterva_new_ctx(void *(*c_alloc)(size_t), void (*c_free)(void *), blosc2_cparams cparams, blosc2_dparams dparams) {
    caterva_ctx_t *ctx;
    ctx = (caterva_ctx_t *) malloc(sizeof(caterva_ctx_t));
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
    free(ctx);
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


static int32_t serialize_meta(int8_t ndim, int64_t *shape, const int32_t *pshape, uint8_t **smeta) {
    // Allocate space for Caterva metalayer
    int32_t max_smeta_len = 1 + 1 + 1 + (1 + ndim * (1 + sizeof(int64_t))) + \
        (1 + ndim * (1 + sizeof(int32_t))) + (1 + ndim * (1 + sizeof(int32_t)));
    *smeta = malloc((size_t)max_smeta_len);
    uint8_t *pmeta = *smeta;

    // Build an array with 5 entries (version, ndim, shape, pshape, bshape)
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

    // pshape entry
    *pmeta++ = (uint8_t)(0x90) + ndim;  // fix array with ndim elements
    for (int8_t i = 0; i < ndim; i++) {
        *pmeta++ = 0xd2;  // int32
        swap_store(pmeta, pshape + i, sizeof(int32_t));
        pmeta += sizeof(int32_t);
    }
    assert(pmeta - *smeta <= max_smeta_len);

    // bshape entry
    *pmeta++ = (uint8_t)(0x90) + ndim;  // fix array with ndim elements
    int32_t *bshape = malloc(CATERVA_MAXDIM * sizeof(int32_t));
    for (int8_t i = 0; i < ndim; i++) {
        *pmeta++ = 0xd2;  // int32
        bshape[i] = 0;  // FIXME: update when support for multidimensional bshapes would be ready
        // NOTE: we need to initialize the header so as to avoid false negatives in valgrind
        swap_store(pmeta, bshape + i, sizeof(int32_t));
        pmeta += sizeof(int32_t);
    }
    free(bshape);
    assert(pmeta - *smeta <= max_smeta_len);
    int32_t slen = (int32_t)(pmeta - *smeta);

    return slen;
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
            carr->pshape[i] = (int32_t)(pshape->dims[i]);
            carr->shape[i] = 1;
            carr->eshape[i] = 1;
            carr->psize *= carr->pshape[i];
        }

        blosc2_schunk *sc = blosc2_new_schunk(ctx->cparams, ctx->dparams, frame);
        if (frame != NULL) {
            // Serialize the dimension info in the associated frame
            if (sc->nmetalayers >= BLOSC2_MAX_METALAYERS) {
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
            int retcode = blosc2_add_metalayer(sc, "caterva", smeta, (uint32_t)smeta_len);
            if (retcode < 0) {
                return NULL;
            }
            free(smeta);
        }
        /* Create a schunk (for a frame-disk-backed one, this implies serializing the header on-disk */
        carr->sc = sc;
    } else {
        carr->storage = CATERVA_STORAGE_PLAINBUFFER;
    }

    /* Copy context to caterva_array_t */
    carr->ctx = (caterva_ctx_t *) ctx->alloc(sizeof(caterva_ctx_t));
    memcpy(&carr->ctx[0], &ctx[0], sizeof(caterva_ctx_t));

    carr->empty = true;
    carr->filled = false;
    carr->nparts = 0;
    return carr;
}


caterva_array_t *caterva_from_frame(caterva_ctx_t *ctx, blosc2_frame *frame, bool copy) {
    caterva_array_t *carr = caterva_blosc_from_frame(ctx, frame, copy);
    return carr;
}


caterva_array_t *caterva_from_sframe(caterva_ctx_t *ctx, uint8_t *sframe, int64_t len, bool copy) {
    caterva_array_t *carr = caterva_blosc_from_sframe(ctx, sframe, len, copy);
    return carr;
}


caterva_array_t *caterva_from_file(caterva_ctx_t *ctx, const char *filename, bool copy) {
    caterva_array_t *carr = caterva_blosc_from_file(ctx, filename, copy);
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
    carr->empty = false;
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

        uint8_t *smeta = NULL;
        // Serialize the dimension info ...
        int32_t smeta_len = serialize_meta(carr->ndim, carr->shape, carr->pshape, &smeta);
        if (smeta_len < 0) {
            fprintf(stderr, "error during serializing dims info for Caterva");
            return -1;
        }
        // ... and update it in its metalayer
        if (blosc2_has_metalayer(carr->sc, "caterva") < 0) {
            int retcode = blosc2_add_metalayer(carr->sc, "caterva", smeta, (uint32_t) smeta_len);
            if (retcode < 0) {
                return -1;
            }
        }
        else {
            int retcode = blosc2_update_metalayer(carr->sc, "caterva", smeta, (uint32_t) smeta_len);
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
            carr->pshape[i] = (int32_t)(shape->dims[i]);
            carr->size *= carr->shape[i];
            carr->esize *= carr->eshape[i];
            carr->psize *= carr->pshape[i];
        }
    }

    return 0;
}


int caterva_append(caterva_array_t *carr, void *part, int64_t partsize) {
    if (partsize != (int64_t) carr->psize * carr->ctx->cparams.typesize) {
        return -1;
    }
    if (carr->filled) {
        return -2;
    }

    switch (carr->storage) {
        case CATERVA_STORAGE_BLOSC:
            caterva_blosc_append(carr, part, partsize);
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            caterva_plainbuffer_append(carr, part, partsize);
            break;
    }

    carr->nparts++;
    if (carr->nparts == carr->esize / carr->psize) {
        carr->filled = true;
    }

    return 0;
}


int caterva_from_buffer(caterva_array_t *dest, caterva_dims_t *shape, const void *src) {
    caterva_update_shape(dest, shape);

    switch (dest->storage) {
        case CATERVA_STORAGE_BLOSC:
            caterva_blosc_from_buffer(dest, shape, src);
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            caterva_plainbuffer_from_buffer(dest, shape, src);
            break;
    }

    return 0;
}


int caterva_to_buffer(caterva_array_t *src, void *dest) {
    switch (src->storage) {
        case CATERVA_STORAGE_BLOSC:
            caterva_blosc_to_buffer(src, dest);
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            caterva_plainbuffer_to_buffer(src, dest);
            break;
    }
    return 0;
}

int caterva_get_slice_buffer(void *dest, caterva_array_t *src, caterva_dims_t *start,
                             caterva_dims_t *stop, caterva_dims_t *d_pshape) {
    switch (src->storage) {
        case CATERVA_STORAGE_BLOSC:
            caterva_blosc_get_slice_buffer(dest, src, start, stop, d_pshape);
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            caterva_plainbuffer_get_slice_buffer(dest, src, start, stop, d_pshape);
            break;
    }
    return 0;
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
    switch (dest->storage) {
        case CATERVA_STORAGE_BLOSC:
            return -1;
        case CATERVA_STORAGE_PLAINBUFFER:
            caterva_plainbuffer_set_slice_buffer(dest, src, start, stop);
            break;
    }
    return 0;
}


int caterva_get_slice(caterva_array_t *dest, caterva_array_t *src, caterva_dims_t *start,
                                                                   caterva_dims_t *stop) {
    if (start->ndim != stop->ndim) {
        return -1;
    }
    if (start->ndim != src->ndim) {
        return -1;
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
    caterva_update_shape(dest, &shape);

    switch (dest->storage) {
        case CATERVA_STORAGE_BLOSC:
            caterva_blosc_get_slice(dest, src, start, stop);
            break;
        case CATERVA_STORAGE_PLAINBUFFER:
            caterva_plainbuffer_get_slice(dest, src, start, stop);
            break;
    }

    return 0;
}

int caterva_repart(caterva_array_t *dest, caterva_array_t *src) {
    if (src->storage != CATERVA_STORAGE_BLOSC) {
        return -1;
    }
    if (dest->storage != CATERVA_STORAGE_BLOSC) {
        return -1;
    }
    int64_t start_[CATERVA_MAXDIM] = {0, 0, 0, 0, 0, 0, 0, 0};
    caterva_dims_t start = caterva_new_dims(start_, dest->ndim);
    int64_t stop_[CATERVA_MAXDIM];
    for (int i = 0; i < dest->ndim; ++i) {
        stop_[i] = src->shape[i];
    }
    caterva_dims_t stop = caterva_new_dims(stop_, dest->ndim);
    caterva_get_slice(dest, src, &start, &stop);
    return 0;
}

int caterva_squeeze(caterva_array_t *src) {
    uint8_t nones = 0;
    int64_t newshape_[CATERVA_MAXDIM];
    int32_t newpshape_[CATERVA_MAXDIM];

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
    caterva_update_shape(src, &newshape);

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


int caterva_copy(caterva_array_t *dest, caterva_array_t *src) {
    caterva_dims_t shape = caterva_new_dims(src->shape, src->ndim);
    if (src->storage == CATERVA_STORAGE_PLAINBUFFER) {
        if (dest->storage == CATERVA_STORAGE_PLAINBUFFER) {
            caterva_update_shape(dest, &shape);
            dest->buf = malloc((size_t) dest->size * dest->ctx->cparams.typesize);
            memcpy(dest->buf, src->buf, dest->size * dest->ctx->cparams.typesize);
            dest->filled = true;
        } else {
            caterva_from_buffer(dest, &shape, src->buf);
        }
    } else {
        if (dest->storage == CATERVA_STORAGE_PLAINBUFFER) {
            caterva_update_shape(dest, &shape);
            dest->buf = malloc((size_t) dest->size * dest->ctx->cparams.typesize);
            caterva_to_buffer(src, dest->buf);
            dest->filled = true;
        } else {
            caterva_repart(dest, src);
        }
    }
    return 0;
}
