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


caterva_array_t *caterva_empty_array_2(caterva_ctx_t *ctx, blosc2_frame *frame, caterva_dims_t *pshape, caterva_dims_t *spshape) {
    /* Create a caterva_array_t buffer */
    caterva_array_t *carr = (caterva_array_t *) ctx->alloc(sizeof(caterva_array_t));
    carr->size = 1;
    carr->psize = 1;
    carr->esize = 1;
    carr->spsize = 1;
    carr->epsize = 1;

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
            carr->spshape[i] = (int32_t)(spshape->dims[i]);
            carr->shape[i] = 1;
            carr->eshape[i] = 1;
            carr->psize *= carr->pshape[i];
            carr->spsize *= carr->spshape[i];

            if (i < carr->ndim) {
                if (carr->pshape[i] % carr->spshape[i] == 0) {
                    carr->epshape[i] = carr->pshape[i];
                } else {                // para extender le añadimos lo que le falta para ser multiplo de spshape
                    carr->epshape[i] = carr->pshape[i] + carr->spshape[i] - carr->pshape[i] % carr->spshape[i];
                }
            } else {
                carr->epshape[i] = 1;
            }
            carr->epsize *= carr->epshape[i];
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


caterva_array_t *caterva_from_sframe(caterva_ctx_t *ctx, uint8_t *sframe, int64_t len, bool copy) {
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


caterva_array_t *caterva_from_file(caterva_ctx_t *ctx, const char *filename, bool copy) {
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


int caterva_free_ctx(caterva_ctx_t *ctx) {
    free(ctx);
    return 0;
}


int caterva_free_array(caterva_array_t *carr) {
    switch (carr->storage) {
        case CATERVA_STORAGE_BLOSC:
            if (carr->sc != NULL) {
                blosc2_free_schunk(carr->sc);
            }
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
            carr->epshape[i] = shape->dims[i];
            carr->spshape[i] = (int32_t)(shape->dims[i]);
            carr->size *= carr->shape[i];
            carr->esize *= carr->eshape[i];
            carr->psize *= carr->pshape[i];
            carr->epsize *= carr->eshape[i];
            carr->spsize *= carr->pshape[i];
        }
    }

    return 0;
}


int caterva_repart_chunk(int8_t *chunk, int size_chunk, void *src, int size_src, caterva_array_t *carr, caterva_ctx_t *ctx){
    if(ctx != carr->ctx) {
        return -1;
    }
    if (size_chunk != carr->epsize * carr->ctx->cparams.typesize) {
        return -2;
    }

    const int8_t *src_b = (int8_t *) src;
    memset(chunk, 0, size_chunk);
    int32_t d_pshape[CATERVA_MAXDIM];
    int64_t d_epshape[CATERVA_MAXDIM];
    int32_t d_spshape[CATERVA_MAXDIM];
    int8_t d_ndim = carr->ndim;

    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        d_pshape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = carr->pshape[i];
        d_epshape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = carr->epshape[i];
        d_spshape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = carr->spshape[i];
    }

    int64_t aux2[CATERVA_MAXDIM];            // en aux2[0] metemos el num de subparticiones total del chunk
    aux2[7] = d_epshape[7] / d_spshape[7];     // numero de subparticiones del chunk en la dim 7
    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
        aux2[i] = d_epshape[i] / d_spshape[i] * aux2[i + 1];    // num subparticiones de la dim i *
    }                                             // num subparticiones de las siguientes dims

    /* Fill each subpartition buffer */
    int64_t orig[CATERVA_MAXDIM];
    int32_t actual_spsize[CATERVA_MAXDIM];
    for (int64_t sci = 0; sci < carr->epsize / carr->spsize; sci++) {    // sci va de 0 al numero de subchunks del chunk
        /*Calculate the coord. of the subpartition first element */
        orig[7] = sci % (d_epshape[7] / d_spshape[7]) * d_spshape[7];
        for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
            orig[i] = sci % (aux2[i]) / (aux2[i + 1]) * d_spshape[i];   // coord del 1r elem de la subparticion sci en la dim i
        }
        /* Calculate if padding with 0s is needed for this subchunk */
        for (int i = CATERVA_MAXDIM - 1; i >= 0; i--) {  // ------------------- OJO AQUI QUE SE PUEDE LIAR ----------------------
            if (orig[i] + d_spshape[i] > d_spshape[i]) {      // si la 1a pos del subchunk siguiente a sci se pasa del tam del chunk (ultimo subchunk)
                actual_spsize[i] = d_pshape[i] - orig[i];    // el nuevo spsize es la distancia entre la 1a pos de la subpart y el final del chunk
            } else {                                    // si sci no es el ultimo subchunk
                actual_spsize[i] = d_spshape[i];          // no cambiamos el spsize
            }
        }
        int32_t seq_copylen = actual_spsize[7] * carr->ctx->cparams.typesize;     // tamaño de la subparticion en la dim plano X (cuanto copiar desde el
        /* Copy each line of data from src to chunk */                                   // inicio de una linea hacia la drerecha)
        int64_t ii[CATERVA_MAXDIM];     // dentro del subchunk sci, ii[i] recorre sus filas en la dimension i
        for (ii[6] = 0; ii[6] < actual_spsize[6]; ii[6]++) {
            for (ii[5] = 0; ii[5] < actual_spsize[5]; ii[5]++) {
                for (ii[4] = 0; ii[4] < actual_spsize[4]; ii[4]++) {
                    for (ii[3] = 0; ii[3] < actual_spsize[3]; ii[3]++) {
                        for (ii[2] = 0; ii[2] < actual_spsize[2]; ii[2]++) {
                            for (ii[1] = 0; ii[1] < actual_spsize[1]; ii[1]++) {
                                for (ii[0] = 0; ii[0] < actual_spsize[0]; ii[0]++) {
                                    int64_t d_a = d_spshape[7];      // en d_a metemos el tam subpart en la dim 7
                                    int64_t d_coord_f = sci * carr->spsize;          // aqui meteremos lo que se le suma a chunk para llegar al 1r elem vacio
                                    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {     // para cada dim
                                        d_coord_f += ii[i] * d_a;   // le sumamos el num de fila en sci en dim i * tam subpart
                                        d_a *= d_spshape[i];         // le multiplicamos el tam subpart en la dim i
                                    }

                                    int64_t s_coord_f = orig[7];    // aqui meteremos lo que se le suma a src para llegar al 1r elem de la fila
                                    int64_t s_a = d_pshape[7];       // tamaño de la dim 7
                                    for (int i = CATERVA_MAXDIM - 2;
                                        i >= 0; i--) {     // a s_coord_f le sumamos la pos en
                                        s_coord_f += (orig[i] + ii[i]) *
                                                     s_a;   // el chunk i * num elem de las siguientes dim
                                        s_a *= d_pshape[i];          // le multiplicamos el tam del chunk en la dim i
                                    }
                                    memcpy(chunk + d_coord_f * carr->ctx->cparams.typesize,
                                           src_b + s_coord_f * carr->ctx->cparams.typesize,
                                           seq_copylen);    // copia una fila de src_b en chunk
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




int caterva_append(caterva_array_t *carr, void *part, int64_t partsize) {
    if (partsize != (int64_t) carr->psize * carr->ctx->cparams.typesize) {
        return -1;
    }
    if (carr->filled) {
        return -2;
    }

    if (carr->storage == CATERVA_STORAGE_BLOSC) {
        blosc2_schunk_append_buffer(carr->sc, part, partsize);
    } else {
        if (carr->nparts == 0) {
            carr->buf = malloc(carr->size * (size_t) carr->ctx->cparams.typesize);
        }
        int64_t start_[CATERVA_MAXDIM], stop_[CATERVA_MAXDIM];
        for (int i = 0; i < carr->ndim; ++i) {
            start_[i] = 0;
            stop_[i] = start_[i] + carr->pshape[i];
        }
        caterva_dims_t start = caterva_new_dims(start_, carr->ndim);
        caterva_dims_t stop = caterva_new_dims(stop_, carr->ndim);
        caterva_set_slice_buffer(carr, part, &start, &stop);
    }
    carr->nparts++;
    if (carr->nparts == carr->esize / carr->psize) {
        carr->filled = true;
    }

    return 0;
}


int caterva_append_2(caterva_array_t *carr, void *part, int64_t partsize) {
    if (partsize != (int64_t) carr->psize * carr->ctx->cparams.typesize) {
        return -1;
    }
    if (carr->filled) {
        return -2;
    }

    caterva_ctx_t *ctx = carr->ctx;
    int32_t typesize = carr->ctx->cparams.typesize;
    int size_rep = (size_t) carr->epsize * typesize;
    int8_t *rep = ctx->alloc(size_rep);
    caterva_repart_chunk(rep, size_rep, part, partsize, carr, ctx);

    if (carr->storage == CATERVA_STORAGE_BLOSC) {
        blosc2_schunk_append_buffer(carr->sc, rep, partsize);
    } else {                                                    // NOOOOOOOOOOOOOOOOOOO
        if (carr->nparts == 0) {
            carr->buf = malloc(carr->size * (size_t) carr->ctx->cparams.typesize);
        }
        int64_t start_[CATERVA_MAXDIM], stop_[CATERVA_MAXDIM];
        for (int i = 0; i < carr->ndim; ++i) {
            start_[i] = 0;
            stop_[i] = start_[i] + carr->pshape[i];
        }
        caterva_dims_t start = caterva_new_dims(start_, carr->ndim);
        caterva_dims_t stop = caterva_new_dims(stop_, carr->ndim);
        caterva_set_slice_buffer(carr, part, &start, &stop);
    }
    carr->nparts++;
    if (carr->nparts == carr->esize / carr->psize) {
        carr->filled = true;
    }

    return 0;
}


int caterva_from_buffer(caterva_array_t *dest, caterva_dims_t *shape, const void *src) {
    const int8_t *src_b = (int8_t *) src;
    caterva_update_shape(dest, shape);

    if (dest->storage == CATERVA_STORAGE_BLOSC) {
        if (dest->sc->nbytes > 0) {
            printf("Caterva container must be empty!");
            return -1;
        }
        int64_t d_shape[CATERVA_MAXDIM];
        int64_t d_eshape[CATERVA_MAXDIM];
        int32_t d_pshape[CATERVA_MAXDIM];
        int8_t d_ndim = dest->ndim;

        for (int i = 0; i < CATERVA_MAXDIM; ++i) {
            d_shape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = dest->shape[i];
            d_eshape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = dest->eshape[i];
            d_pshape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = dest->pshape[i];
        }

        caterva_ctx_t *ctx = dest->ctx;
        int32_t typesize = dest->sc->typesize;
        int8_t *chunk = ctx->alloc((size_t) dest->psize * typesize);

        /* Calculate the constants out of the for  */
        int64_t aux[CATERVA_MAXDIM];
        aux[7] = d_eshape[7] / d_pshape[7];
        for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
            aux[i] = d_eshape[i] / d_pshape[i] * aux[i + 1];
        }

        /* Fill each chunk buffer */
        int64_t desp[CATERVA_MAXDIM];
        int32_t actual_psize[CATERVA_MAXDIM];
        for (int64_t ci = 0; ci < dest->esize / dest->psize; ci++) {
            memset(chunk, 0, dest->psize * typesize);
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
                                        memcpy(chunk + d_coord_f * typesize, src_b + s_coord_f * typesize, seq_copylen);
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
    } else {
        // Plain buffer
        if (dest->buf != NULL) {
            printf("Caterva container must be empty!");
            return -1;
        }
        dest->buf = malloc(dest->size * (size_t) dest->ctx->cparams.typesize);
        memcpy(dest->buf, src, dest->size * (size_t) dest->ctx->cparams.typesize);
    }
    dest->filled = true;
    return 0;
}

int caterva_from_buffer_2(caterva_array_t *dest, caterva_dims_t *shape, const void *src) {
    /*printf("\n buffer en doubles: \n", NULL);
    for (int i = 0; i < dest->psize; i++){
        printf("%f,", ((double*) src)[i]);
    }*/
    const int8_t *src_b = (int8_t *) src;
    caterva_update_shape(dest, shape);
    /*printf("\n buffer inicial: \n", NULL);
    for (int i = 0; i < dest->psize; i++){
        printf("%f,",  ((double*) src_b)[i]);
    }*/
    if (dest->storage == CATERVA_STORAGE_BLOSC) {
        if (dest->sc->nbytes > 0) {
            printf("Caterva container must be empty!");
            return -1;
        }
        int64_t d_shape[CATERVA_MAXDIM];
        int64_t d_eshape[CATERVA_MAXDIM];
        int32_t d_pshape[CATERVA_MAXDIM];
        int8_t d_ndim = dest->ndim;

        for (int i = 0; i < CATERVA_MAXDIM; ++i) {
            d_shape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = dest->shape[i];
            d_eshape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = dest->eshape[i];
            d_pshape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = dest->pshape[i];
        }

        caterva_ctx_t *ctx = dest->ctx;
        int32_t typesize = dest->sc->typesize;
        int8_t *chunk = ctx->alloc((size_t) dest->psize * typesize);
        int8_t *rchunk = ctx->alloc((size_t) dest->epsize * typesize);

        /* Calculate the constants out of the for  */
        int64_t aux[CATERVA_MAXDIM];
        aux[7] = d_eshape[7] / d_pshape[7];
        for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
            aux[i] = d_eshape[i] / d_pshape[i] * aux[i + 1];
        }

        /* Fill each chunk buffer */
        int64_t desp[CATERVA_MAXDIM];
        int32_t actual_psize[CATERVA_MAXDIM];
        for (int64_t ci = 0; ci < dest->esize / dest->psize; ci++) {
            memset(chunk, 0, dest->psize * typesize);
            memset(rchunk, 0, dest->epsize * typesize);

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
                                        memcpy(chunk + d_coord_f * typesize, src_b + s_coord_f * typesize, seq_copylen);
                                    }
                                }
                            }
                        }
                    }
                }
            }
          /*  printf("\n buffer del chunk: \n", NULL);
            for (int i = 0; i < dest->psize; i++){
                printf("%f,",  ((double*) chunk)[i]);
            }*/
            caterva_repart_chunk(rchunk, (size_t) dest->epsize * typesize, chunk, (size_t) dest->psize * typesize, dest, ctx);
            printf("\n buffer del rchunk: \n", NULL);
            for (int i = 0; i < dest->epsize; i++){
                printf("%f,",  ((double*) rchunk)[i]);
            }
            blosc2_schunk_append_buffer(dest->sc, rchunk, (size_t) dest->epsize * typesize);
            printf("\n Append: \n", NULL);
            for( int i = 0; i < dest->epsize; i++){     // ESTA TODO COMPRIMIDO?????? POR ESO TODO 0??????
                printf("%f,", ((double*) dest->sc)[ci * dest->epsize + i]);
            }
        }
        ctx->free(chunk);
        ctx->free(rchunk);
    } else {
        // Plain buffer
        if (dest->buf != NULL) {
            printf("Caterva container must be empty!");
            return -1;
        }
        dest->buf = malloc(dest->size * (size_t) dest->ctx->cparams.typesize);
        memcpy(dest->buf, src, dest->size * (size_t) dest->ctx->cparams.typesize);
    }
    dest->filled = true;
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
    }
    return 0;
}


int caterva_get_slice_buffer_2(void *dest, caterva_array_t *src, caterva_dims_t *start,
                             caterva_dims_t *stop, caterva_dims_t *d_pshape) {
    uint8_t *bdest = dest;   // for allowing pointer arithmetic
    int64_t start_[CATERVA_MAXDIM];
    int64_t stop_[CATERVA_MAXDIM];
    int64_t d_pshape_[CATERVA_MAXDIM];
    int64_t s_pshape[CATERVA_MAXDIM];
    int64_t s_eshape[CATERVA_MAXDIM];
    int64_t s_spshape[CATERVA_MAXDIM];
    int64_t s_epshape[CATERVA_MAXDIM];
    int8_t s_ndim = src->ndim;

    if (src->storage == CATERVA_STORAGE_BLOSC) {
        for (int i = 0; i < CATERVA_MAXDIM; ++i) {
            start_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = start->dims[i];
            stop_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = stop->dims[i];
            d_pshape_[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = d_pshape->dims[i];
            s_eshape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->eshape[i];
            s_pshape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->pshape[i];
            s_epshape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->epshape[i];
            s_spshape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->spshape[i];
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
        /*bool local_cache;
        if (src->part_cache.data == NULL) {
            chunk = (uint8_t *) ctx->alloc((size_t) src->psize * typesize);
            local_cache = true;
        } else {
            chunk = src->part_cache.data;
            local_cache = false;
        }*/
        uint8_t *spart;
        spart = (uint8_t *) ctx->alloc((size_t) src->spsize * typesize);
        int64_t i_start[CATERVA_MAXDIM], i_stop[CATERVA_MAXDIM];
        for (int i = 0; i < CATERVA_MAXDIM; ++i) {
            i_start[i] = start_[i] / s_pshape[i];
            i_stop[i] = (stop_[i] - 1) / s_pshape[i];
        }

        /* Calculate the used chunks */
        bool needs_free;
        int64_t ii[CATERVA_MAXDIM], jj[CATERVA_MAXDIM], kk[CATERVA_MAXDIM];
        int64_t j_start[CATERVA_MAXDIM], j_stop[CATERVA_MAXDIM];
        int64_t sp_start[CATERVA_MAXDIM], sp_stop[CATERVA_MAXDIM];
        for (ii[0] = i_start[0]; ii[0] <= i_stop[0]; ++ii[0]) {
            for (ii[1] = i_start[1]; ii[1] <= i_stop[1]; ++ii[1]) {
                for (ii[2] = i_start[2]; ii[2] <= i_stop[2]; ++ii[2]) {
                    for (ii[3] = i_start[3]; ii[3] <= i_stop[3]; ++ii[3]) {
                        for (ii[4] = i_start[4]; ii[4] <= i_stop[4]; ++ii[4]) {
                            for (ii[5] = i_start[5]; ii[5] <= i_stop[5]; ++ii[5]) {
                                for (ii[6] = i_start[6]; ii[6] <= i_stop[6]; ++ii[6]) {
                                    for (ii[7] = i_start[7]; ii[7] <= i_stop[7]; ++ii[7]) {
                                        /* Get the chunk ii */
                                        int nchunk = 0;
                                        int inc = 1;
                                        for (int i = CATERVA_MAXDIM - 1; i >= 0; --i) {
                                            nchunk += (int) (ii[i] * inc);
                                            inc *= (int) (s_eshape[i] / s_pshape[i]);
                                        }

                                        if ((src->part_cache.data == NULL) || (src->part_cache.nchunk != nchunk)) {
                                            blosc2_schunk_get_chunk(src->sc, nchunk, &chunk, &needs_free);
                                            for (int i = 0; i < src->psize; i++){
                                                printf("Chunk: pos %d, value %f, \n", i, ((double*) chunk)[i]);
                                            }
                                        }
                                        if (src->part_cache.data != NULL) {
                                            src->part_cache.nchunk = nchunk;
                                        }

                                        /* Calculate the used subpartitions */
                                        for (int i = 0; i < CATERVA_MAXDIM; ++i) {
                                            if (ii[i] == i_start[i]) {
                                                j_start[i] = (start_[i] % s_pshape[i]) / s_spshape[i];
                                            } else {
                                                j_start[i] = 0;
                                            }
                                            if (ii[i] == i_stop[i]) {
                                                j_stop[i] = ((stop_[i]-1) % s_pshape[i]) / s_spshape[i];
                                            } else {
                                                j_stop[i] = (s_epshape[i] / s_spshape[i]) - 1;
                                            }
                                        }
                                        for (jj[0] = j_start[0]; jj[0] <= j_stop[0]; ++jj[0]) {
                                            for (jj[1] = j_start[1]; jj[1] <= j_stop[1]; ++jj[1]) {
                                                for (jj[2] = j_start[2]; jj[2] <= j_stop[2]; ++jj[2]) {
                                                    for (jj[3] = j_start[3]; jj[3] <= j_stop[3]; ++jj[3]) {
                                                        for (jj[4] = j_start[4]; jj[4] <= j_stop[4]; ++jj[4]) {
                                                            for (jj[5] = j_start[5]; jj[5] <= j_stop[5]; ++jj[5]) {
                                                                for (jj[6] = j_start[6]; jj[6] <= j_stop[6]; ++jj[6]) {
                                                                    for (jj[7] = j_start[7]; jj[7] <= j_stop[7]; ++jj[7]) {
                                                                        /* Decompress subpartition jj */
                                                                        int s_start = 0;
                                                                        int sinc = 1;
                                                                        for (int i = CATERVA_MAXDIM - 1; i >= 0; --i) {
                                                                            s_start += (int) (jj[i] * s_spshape[i] * sinc);
                                                                            sinc *= (int) s_epshape[i];                      // EPSHAPE ???
                                                                        }
                                                                        blosc_getitem(chunk, s_start, src->spsize, spart);
                                                                        for( int i = 0; i< src->spsize; i++){
                                                                            printf("Spart: pos %d, value %f, \n", i, ((double*) spart)[i]);
                                                                        }

                                                                        /* memcpy */
                                                                        for (int i = 0; i < CATERVA_MAXDIM; ++i) {      // para cada dimension
                                                                            if (jj[i] == j_start[i] && ii[i] == i_start[i]) {       // si estamos en la primera subparticion del slice
                                                                                sp_start[i] = (start_[i] % s_pshape[i]) % s_spshape[i];   // sp_start es donde empieza el slice en la spart
                                                                            } else {                                  // si estamos en otra particion
                                                                                sp_start[i] = 0;                       // empezara directamente en el primer elemento de la particion
                                                                            }
                                                                            if (jj[i] == j_stop[i] && ii[i] == i_stop[i]) {      // si estamos en la ultima particion del slice
                                                                                sp_stop[i] = (((stop_[i] - 1) % s_pshape[i]) % s_spshape[i]) + 1;     // sp_stop es las pos que se pasa el stop del principio esta particion
                                                                            } else {                                 // si estamos en otra particion
                                                                                sp_stop[i] = s_spshape[i];             // sp_stop es el ultimo elemento de esta particion
                                                                            }
                                                                        }

                                                                        kk[7] = sp_start[7];
                                                                        for (kk[0] = sp_start[0]; kk[0] < sp_stop[0]; ++kk[0]) {
                                                                            for (kk[1] = sp_start[1]; kk[1] < sp_stop[1]; ++kk[1]) {
                                                                                for (kk[2] = sp_start[2]; kk[2] < sp_stop[2]; ++kk[2]) {
                                                                                    for (kk[3] = sp_start[3]; kk[3] < sp_stop[3]; ++kk[3]) {
                                                                                        for (kk[4] = sp_start[4]; kk[4] < sp_stop[4]; ++kk[4]) {
                                                                                            for (kk[5] = sp_start[5]; kk[5] < sp_stop[5]; ++kk[5]) {
                                                                                                for (kk[6] = sp_start[6]; kk[6] < sp_stop[6]; ++kk[6]) {
                                                                                                    // check we are not in an empty line (padding)
                                                                                                    bool empty_line = false;
                                                                                                    for (int i = 0; i < 7; i++) {
                                                                                                        if ((jj[i] * s_spshape[i] + kk[i]) >= s_pshape[i]) {
                                                                                                            empty_line = true;
                                                                                                            break;
                                                                                                        }
                                                                                                    }
                                                                                                    if (! empty_line) {
                                                                                                        if ((jj[7] + 1) * s_spshape[7] > s_pshape[7]) { // case padding in dim7
                                                                                                            int64_t lastn = s_pshape[7] % s_spshape[7];
                                                                                                            if (lastn < sp_stop[7]) {
                                                                                                                sp_stop[7] = lastn;
                                                                                                            }
                                                                                                        }

                                                                                                        int64_t sp_pointer = 0;
                                                                                                        int64_t sp_pointer_inc = 1;
                                                                                                        for (int i = CATERVA_MAXDIM - 1; i >= 0; --i) {
                                                                                                            sp_pointer += kk[i] * sp_pointer_inc;
                                                                                                            sp_pointer_inc *= s_spshape[i];
                                                                                                        }
                                                                                                        int64_t buf_pointer = 0;
                                                                                                        int64_t buf_pointer_inc = 1;
                                                                                                        for (int i = CATERVA_MAXDIM - 1; i >= 0; --i) {
                                                                                                            buf_pointer += (kk[i] + s_spshape[i] * jj[i] + s_pshape[i] * ii[i] -
                                                                                                                            start_[i]) * buf_pointer_inc;
                                                                                                            buf_pointer_inc *= d_pshape_[i];
                                                                                                        }
                                                                                                        memcpy(&bdest[buf_pointer * typesize],
                                                                                                               &spart[sp_pointer * typesize],
                                                                                                               (sp_stop[7] - sp_start[7]) * typesize);
                                                                                                       /* for(int64_t i = 0; i < sp_stop[7] - sp_start[i]; i++) {
                                                                                                            printf("%f - \n", (double) spart[(sp_pointer + i) * typesize]);
                                                                                                            printf("%f,\n", (double) bdest[(buf_pointer + i) * typesize]);

                                                                                                        }*/
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
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        /*if (local_cache) {
            ctx->free(chunk);
        }*/
        if(needs_free){
            ctx->free(chunk);
        }
    } else {
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
    caterva_update_shape(dest, &shape);

    if (dest->storage == CATERVA_STORAGE_BLOSC) {
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
        caterva_get_slice_buffer(dest->buf, src, start, stop, &shape);
    }
    dest->filled = true;
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
