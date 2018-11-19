/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#include "caterva.h"
#include "assert.h"

caterva_ctxt *caterva_new_ctxt(void *(*c_alloc)(size_t), void (*c_free)(void *)) {
    caterva_ctxt *ctxt;
    if (c_alloc == NULL) {
        ctxt = (caterva_ctxt *) malloc(sizeof(caterva_ctxt));
        ctxt->alloc = malloc;
    } else {
        ctxt = (caterva_ctxt *) c_alloc(sizeof(caterva_ctxt));
        ctxt->alloc = c_alloc;
    }
    if (c_free == NULL) {
        ctxt->free = free;
    } else {
        ctxt->free = c_free;
    }
    return ctxt;
}

caterva_pparams caterva_new_pparams(size_t *shape, size_t *pshape, size_t ndim) {
    caterva_pparams pparams = CATERVA_PPARAMS_ONES;
    for (size_t i = 0; i < ndim; ++i) {
        pparams.shape[CATERVA_MAXDIM - ndim + i] = shape[i];
        pparams.pshape[CATERVA_MAXDIM - ndim + i] = pshape[i];
    }
    pparams.ndim = ndim;
    return pparams;
}

// Serialize the partition params
int32_t serialize_attrs(caterva_pparams pparams, uint8_t **sattrs) {
    int8_t ndim = (int8_t)pparams.ndim;
    int32_t max_sattrs_size = 4096;
    *sattrs = malloc((size_t)max_sattrs_size);
    uint8_t *pattrs = *sattrs;

    // Build a map with 3 entries (ndim, shape, pshape)
    *pattrs++ = 0x80 + 3;

    // ndim entry
    *pattrs++ = (uint8_t)(0b101 << 5) + (uint8_t)strlen("ndim");
    memcpy(pattrs, "ndim", strlen("ndim"));
    pattrs += strlen("ndim");
    *pattrs++ = 0xcc;  // uint8
    memcpy(pattrs, &ndim, sizeof(uint8_t));
    pattrs += sizeof(uint8_t);
    assert(pattrs - *sattrs < max_sattrs_size);

    // shape entry
    *pattrs++ = (uint8_t)(0b101 << 5) + (uint8_t)strlen("shape");
    memcpy(pattrs, "shape", strlen("shape"));
    pattrs += strlen("shape");
    *pattrs++ = (uint8_t)(0b1001 << 4) + (uint8_t)ndim;  // fix array with ndim elements
    for (int8_t i = 0; i < ndim; i++) {
        *pattrs++ = 0xcf;  // uint64
        memcpy(pattrs, &(pparams.shape[i]), sizeof(uint64_t));
        pattrs += sizeof(uint64_t);
    }
    assert(pattrs - *sattrs < max_sattrs_size);

    // pshape entry
    *pattrs++ = (uint8_t)(0b101 << 5) + (uint8_t)strlen("pshape");
    memcpy(pattrs, "pshape", strlen("pshape"));
    pattrs += strlen("shape");
    *pattrs++ = (uint8_t)(0b1001 << 4) + (uint8_t)ndim;  // fix array with ndim elements
    for (int8_t i = 0; i < ndim; i++) {
        *pattrs++ = 0xcf;  // uint64
        memcpy(pattrs, &(pparams.pshape[i]), sizeof(uint64_t));
        pattrs += sizeof(uint64_t);
    }
    assert(pattrs - *sattrs < max_sattrs_size);

    int32_t slen = (int32_t)(pattrs - *sattrs);
    *sattrs = realloc(*sattrs, (size_t)slen);  // get rid of the excess of bytes allocated

    return slen;
}

caterva_array *caterva_new_array(blosc2_cparams cp, blosc2_dparams dp, blosc2_frame *fp,
    caterva_pparams pparams, caterva_ctxt *ctxt) {
    uint8_t *sattrs = NULL;

    /* Create a caterva_array buffer */
    caterva_array *carr = (caterva_array *) ctxt->alloc(sizeof(caterva_array));

    if (fp != NULL) {
        if (fp->nclients >= BLOSC2_MAX_FRAME_CLIENTS) {
            fprintf(stderr, "the number of clients for this frame has been exceeded\n");
            return NULL;
        }
        // Serialize the system attributes for caterva as a client
        int32_t sattrs_len = serialize_attrs(pparams, &sattrs);
        if (sattrs_len < 0) {
            fprintf(stderr, "error during serializing attrs for Caterva");
            return NULL;
        }
        blosc2_frame_attrs *attrs = malloc(sizeof(blosc2_frame_attrs));
        attrs->namespace = strdup("caterva");
        attrs->sattrs = sattrs;
        attrs->sattrs_len = sattrs_len;
        fp->attrs[fp->nclients] = attrs;
        fp->nclients++;
    }

    /* Create a schunk */
    blosc2_schunk *sc = blosc2_new_schunk(cp, dp, fp);
    carr->sc = sc;
    carr->size = 1;
    carr->csize = 1;
    carr->esize = 1;
    carr->ndim = pparams.ndim;

    for (unsigned int i = 0; i < CATERVA_MAXDIM; i++) {
        carr->shape[i] = pparams.shape[i];
        carr->pshape[i] = pparams.pshape[i];

        if (i >= CATERVA_MAXDIM - pparams.ndim) {
            if (pparams.shape[i] % pparams.pshape[i] == 0) {
                carr->eshape[i] = pparams.shape[i];
            } else {
                carr->eshape[i] = pparams.shape[i] + pparams.pshape[i] - pparams.shape[i] % pparams.pshape[i];
            }
        } else {
            carr->eshape[i] = 1;
        }
        carr->size *= carr->shape[i];
        carr->csize *= carr->pshape[i];
        carr->esize *= carr->eshape[i];
        carr->ctxt = (caterva_ctxt *) ctxt->alloc(sizeof(caterva_ctxt));
        memcpy(&carr->ctxt[0], &ctxt[0], sizeof(caterva_ctxt));
    }
    return carr;
}

int caterva_free_ctxt(caterva_ctxt *ctxt) {
    ctxt->free(ctxt);
    return 0;
}

int caterva_free_array(caterva_array *carr) {
    blosc2_free_schunk(carr->sc);
    void (*aux_free)(void *) = carr->ctxt->free;
    caterva_free_ctxt(carr->ctxt);
    aux_free(carr);
    return 0;
}

// Fill a caterva array from a C buffer
// The caterva array must be empty at the begining
int caterva_from_buffer(caterva_array *dest, void *src) {
    int8_t *s_b = (int8_t *) src;
    if (dest->sc->nbytes > 0) {
        printf("Caterva container must be empty!");
        return -1;
    }

    caterva_ctxt *ctxt = dest->ctxt;
    int typesize = dest->sc->typesize;
    int8_t *chunk = ctxt->alloc(dest->csize * typesize);

    /* Calculate the constants out of the for  */
    size_t aux[CATERVA_MAXDIM];
    aux[7] = dest->eshape[7] / dest->pshape[7];
    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
        aux[i] = dest->eshape[i] / dest->pshape[i] * aux[i + 1];
    }

    /* Fill each chunk buffer */
    size_t desp[CATERVA_MAXDIM], r[CATERVA_MAXDIM];
    for (size_t ci = 0; ci < dest->esize / dest->csize; ci++) {
        memset(chunk, 0, dest->csize * typesize);

        /* Calculate the coord. of the chunk first element */
        desp[7] = ci % (dest->eshape[7] / dest->pshape[7]) * dest->pshape[7];
        for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
            desp[i] = ci % (aux[i]) / (aux[i + 1]) * dest->pshape[i];
        }

        /* Calculate if pad with 0 are needed in this chunk */
        for (int i = CATERVA_MAXDIM - 1; i >= 0; i--) {
            if (desp[i] + dest->pshape[i] > dest->shape[i]) {
                r[i] = dest->shape[i] - desp[i];
            } else {
                r[i] = dest->pshape[i];
            }
        }

        /* Copy each line of data from chunk to arr */
        size_t s_coord_f, d_coord_f, s_a, d_a;
        size_t ii[CATERVA_MAXDIM];
        for (ii[6] = 0; ii[6] < r[6]; ii[6]++) {
            for (ii[5] = 0; ii[5] < r[5]; ii[5]++) {
                for (ii[4] = 0; ii[4] < r[4]; ii[4]++) {
                    for (ii[3] = 0; ii[3] < r[3]; ii[3]++) {
                        for (ii[2] = 0; ii[2] < r[2]; ii[2]++) {
                            for (ii[1] = 0; ii[1] < r[1]; ii[1]++) {
                                for (ii[0] = 0; ii[0] < r[0]; ii[0]++) {
                                    d_coord_f = 0;
                                    d_a = dest->pshape[7];

                                    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
                                        d_coord_f += ii[i] * d_a;
                                        d_a *= dest->pshape[i];
                                    }

                                    s_coord_f = desp[7];
                                    s_a = dest->shape[7];

                                    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
                                        s_coord_f += (desp[i] + ii[i]) * s_a;
                                        s_a *= dest->shape[i];
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
        blosc2_schunk_append_buffer(dest->sc, chunk, dest->csize * typesize);
    }

    ctxt->free(chunk);
    return 0;
}

int caterva_to_buffer(caterva_array *src, void *dest) {
    int8_t *d_b = (int8_t *) dest;

    /* Initialise a chunk buffer */
    caterva_ctxt *ctxt = src->ctxt;
    int typesize = src->sc->typesize;
    int8_t *chunk = (int8_t *) ctxt->alloc(src->csize * typesize);

    /* Calculate the constants out of the for  */
    size_t aux[CATERVA_MAXDIM];
    aux[7] = src->eshape[7] / src->pshape[7];
    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
        aux[i] = src->eshape[i] / src->pshape[i] * aux[i + 1];
    }

    /* Fill array from schunk (chunk by chunk) */
    size_t desp[CATERVA_MAXDIM], r[CATERVA_MAXDIM];
    for (size_t ci = 0; ci < src->esize / src->csize; ci++) {
        blosc2_schunk_decompress_chunk(src->sc, (int) ci, chunk, src->csize * typesize);

        /* Calculate the coord. of the chunk first element in arr buffer */
        desp[7] = ci % aux[7] * src->pshape[7];
        for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
            desp[i] = ci % (aux[i]) / (aux[i + 1]) * src->pshape[i];
        }

        /* Calculate if pad with 0 are needed in this chunk */
        for (int i = CATERVA_MAXDIM - 1; i >= 0; i--) {
            if (desp[i] + src->pshape[i] > src->shape[i]) {
                r[i] = src->shape[i] - desp[i];
            } else {
                r[i] = src->pshape[i];
            }
        }

        /* Copy each line of data from chunk to arr */
        size_t s_coord_f, d_coord_f, s_a, d_a;
        size_t ii[CATERVA_MAXDIM];
        for (ii[6] = 0; ii[6] < r[6]; ii[6]++) {
            for (ii[5] = 0; ii[5] < r[5]; ii[5]++) {
                for (ii[4] = 0; ii[4] < r[4]; ii[4]++) {
                    for (ii[3] = 0; ii[3] < r[3]; ii[3]++) {
                        for (ii[2] = 0; ii[2] < r[2]; ii[2]++) {
                            for (ii[1] = 0; ii[1] < r[1]; ii[1]++) {
                                for (ii[0] = 0; ii[0] < r[0]; ii[0]++) {
                                    s_coord_f = 0;
                                    s_a = src->pshape[7];

                                    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
                                        s_coord_f += ii[i] * s_a;
                                        s_a *= src->pshape[i];
                                    }

                                    d_coord_f = desp[7];
                                    d_a = src->shape[7];

                                    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
                                        d_coord_f += (desp[i] + ii[i]) * d_a;
                                        d_a *= src->shape[i];
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

    ctxt->free(chunk);
    return 0;
}

int caterva_get_slice(caterva_array *src, void *dest, size_t *start, size_t *stop) {

    /* Create chunk buffers */
    caterva_ctxt *ctxt = src->ctxt;
    int typesize = src->sc->typesize;
    uint8_t *chunk = (uint8_t *) ctxt->alloc(src->csize * typesize);

    /* Reformat start and stop */
    size_t start_aux[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    size_t stop_aux[8] = {1, 1, 1, 1, 1, 1, 1, 1};
    for (size_t i = 0; i < src->ndim; ++i) {
        start_aux[CATERVA_MAXDIM - src->ndim + i] = start[i];
        stop_aux[CATERVA_MAXDIM - src->ndim + i] = stop[i];
    }
    stop = stop_aux;
    start = start_aux;

    size_t i_start[8], i_stop[8];
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        i_start[i] = start[i] / src->pshape[i];
        i_stop[i] = (stop[i] - 1) / src->pshape[i];
    }

    /* Calculate the used chunks */
    size_t ii[CATERVA_MAXDIM], jj[CATERVA_MAXDIM];
    size_t c_start[CATERVA_MAXDIM], c_stop[CATERVA_MAXDIM];
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
                                        nchunk += ii[i] * inc;
                                        inc *= src->eshape[i] / src->pshape[i];
                                    }
                                    blosc2_schunk_decompress_chunk(src->sc, nchunk, chunk, src->csize * typesize);
                                    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
                                        if (ii[i] == (start[i] / src->pshape[i])) {
                                            c_start[i] = start[i] % src->pshape[i];
                                        } else {
                                            c_start[i] = 0;
                                        }
                                        if (ii[i] == stop[i] / src->pshape[i]) {
                                            c_stop[i] = stop[i] % src->pshape[i];
                                        } else {
                                            c_stop[i] = src->pshape[i];
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
                                                                size_t chunk_pointer = 0;
                                                                size_t chunk_pointer_inc = 1;
                                                                for (int i = CATERVA_MAXDIM - 1; i >= 0; --i) {
                                                                    chunk_pointer += jj[i] * chunk_pointer_inc;
                                                                    chunk_pointer_inc *= src->pshape[i];
                                                                }
                                                                size_t buf_pointer = 0;
                                                                size_t buf_pointer_inc = 1;
                                                                for (int i = CATERVA_MAXDIM - 1; i >= 0; --i) {
                                                                    buf_pointer += (jj[i] + src->pshape[i] * ii[i] -
                                                                        start[i]) * buf_pointer_inc;
                                                                    buf_pointer_inc *= (stop[i] - start[i]);
                                                                }
                                                                memcpy(&dest[buf_pointer * typesize],
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
    ctxt->free(chunk);

    return 0;
}
