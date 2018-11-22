/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#include <caterva.h>
#include "caterva.h"

caterva_ctx *caterva_new_ctx(void *(*c_alloc)(size_t), void (*c_free)(void *), blosc2_cparams cparams, blosc2_dparams dparams) {
    caterva_ctx *ctx;
    if (c_alloc == NULL) {
        ctx = (caterva_ctx *) malloc(sizeof(caterva_ctx));
        ctx->alloc = malloc;
    } else {
        ctx = (caterva_ctx *) c_alloc(sizeof(caterva_ctx));
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


caterva_dims caterva_new_dims(size_t *dims, size_t ndim) {
    caterva_dims dims_s = CATERVA_DIMS_DEFAULTS;
    for (size_t i = 0; i < ndim; ++i) {
        dims_s.dims[i] = dims[i];
    }
    dims_s.ndim = ndim;
    return dims_s;
}

// Serialize the partition params
//FIXME: Review if that is correct

uint8_t* serialize_attrs(caterva_dims dims_s) {
    int8_t ndim = (int8_t)dims_s.ndim;
    uint8_t* sattrs = malloc(4096);
    uint8_t *pattrs = sattrs;

    // Build a map with 3 entries (ndim, dims)
    *pattrs++ = (0b1000 << 4) + 2;

    // ndim entry
    *pattrs++ = (uint8_t)(0b101 << 5) + (uint8_t)strlen("ndim");
    memcpy(pattrs, "ndim", strlen("ndim"));
    pattrs += strlen("ndim");
    *pattrs++ = 0xcc;  // uint8
    memcpy(pattrs, &ndim, sizeof(uint8_t));
    pattrs += sizeof(uint8_t);

    // shape entry
    *pattrs++ = (uint8_t)(0b101 << 5) + (uint8_t)strlen("dims");
    memcpy(pattrs, "dims", strlen("dims"));
    pattrs += strlen("dims");
    *pattrs++ = (uint8_t)(0b1001 << 4) + (uint8_t)ndim;  // fix array with ndim elements
    for (int8_t i = 0; i < ndim; i++) {
        *pattrs++ = 0xcf;  // uint64
        memcpy(pattrs, &(dims_s.dims[i]), sizeof(uint64_t));
        pattrs += sizeof(uint64_t);
    }
    
    size_t slen = pattrs - sattrs;
    sattrs = realloc(sattrs, slen);  // get rid of the excess of bytes allocated

    return sattrs;
}

caterva_array *caterva_empty_array(caterva_ctx *ctx, blosc2_frame *fp, caterva_dims pshape) {
    uint8_t *sattrs = NULL;

    /* Create a caterva_array buffer */
    caterva_array *carr = (caterva_array *) ctx->alloc(sizeof(caterva_array));
    
    /* Copy context to caterva_array */
    carr->ctx = (caterva_ctx *) ctx->alloc(sizeof(caterva_ctx));
    memcpy(&carr->ctx[0], &ctx[0], sizeof(caterva_ctx));

    if (fp != NULL) {
        if (fp->nclients >= BLOSC2_MAX_FRAME_CLIENTS) {
            fprintf(stderr, "the number of clients for this frame has been exceeded\n");
            return NULL;
        }
        // Serialize the system attributes for caterva as a client
        sattrs = serialize_attrs(pshape);
        blosc2_frame_attrs *attrs = malloc(sizeof(blosc2_frame_attrs));
        attrs->namespace = strdup("caterva");
        attrs->sattrs = sattrs;
        fp->attrs[fp->nclients] = attrs;
        fp->nclients++;
    }

    /* Create a schunk */
    blosc2_schunk *sc = blosc2_new_schunk(ctx->cparams, ctx->dparams, fp);
    carr->sc = sc;
    carr->size = 1;
    carr->csize = 1;
    carr->esize = 1;
    carr->ndim = pshape.ndim;

    for (unsigned int i = 0; i < CATERVA_MAXDIM; i++) {
        carr->pshape[i] = pshape.dims[i];
        carr->shape[i] = 1;
        carr->eshape[i] = 1;
        carr->csize *= carr->pshape[i];
    }
    return carr;
}

int caterva_free_ctx(caterva_ctx *ctx) {
    ctx->free(ctx);
    return 0;
}

int caterva_free_array(caterva_array *carr) {
    blosc2_free_schunk(carr->sc);
    void (*aux_free)(void *) = carr->ctx->free;
    caterva_free_ctx(carr->ctx);
    aux_free(carr);
    return 0;
}

int _caterva_update_shape(caterva_array *carr, caterva_dims shape) {

    if (carr->ndim != shape.ndim) {
        printf("caterva array ndim and shape ndim are not equal\n");
        return -1;
    }
    carr->size = 1;
    carr->esize = 1;
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        carr->shape[i] = shape.dims[i];

        if (i < shape.ndim) {
            if (shape.dims[i] % carr->pshape[i] == 0) {
                carr->eshape[i] = shape.dims[i];
            } else {
                carr->eshape[i] = shape.dims[i] + carr->pshape[i] - shape.dims[i] % carr->pshape[i];
            }
        } else {
            carr->eshape[i] = 1;
        }
        carr->size *= carr->shape[i];
        carr->esize *= carr->eshape[i];
    }

    //FIXME: Serialize the shape here

    return 0;
}

// Fill a caterva array from a C buffer
// The caterva array must be empty at the begining
int caterva_from_buffer(caterva_array *dest, caterva_dims shape, void *src) {
    int8_t *s_b = (int8_t *) src;
    
    if (dest->sc->nbytes > 0) {
        printf("Caterva container must be empty!");
        return -1;
    }

    _caterva_update_shape(dest, shape);

    size_t d_shape[CATERVA_MAXDIM];
    size_t d_pshape[CATERVA_MAXDIM];
    size_t d_eshape[CATERVA_MAXDIM];
    size_t d_ndim = dest->ndim;
    
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        d_shape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = dest->shape[i];
        d_eshape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = dest->eshape[i];
        d_pshape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = dest->pshape[i];
    }
    
    caterva_ctx *ctx = dest->ctx;
    int typesize = dest->sc->typesize;
    int8_t *chunk = malloc(dest->csize * typesize);

    /* Calculate the constants out of the for  */
    size_t aux[CATERVA_MAXDIM];
    aux[7] = d_eshape[7] / d_pshape[7];
    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
        aux[i] = d_eshape[i] / d_pshape[i] * aux[i + 1];
    }

    /* Fill each chunk buffer */
    size_t desp[CATERVA_MAXDIM], r[CATERVA_MAXDIM];
    for (size_t ci = 0; ci < dest->esize / dest->csize; ci++) {
        memset(chunk, 0, dest->csize * typesize);

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
        blosc2_schunk_append_buffer(dest->sc, chunk, dest->csize * typesize);
    }

    ctx->free(chunk);
    return 0;
}

int caterva_to_buffer(caterva_array *src, void *dest) {
    int8_t *d_b = (int8_t *) dest;

    size_t s_shape[CATERVA_MAXDIM];
    size_t s_pshape[CATERVA_MAXDIM];
    size_t s_eshape[CATERVA_MAXDIM];
    size_t s_ndim = src->ndim;

    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        s_shape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->shape[i];
        s_eshape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->eshape[i];
        s_pshape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->pshape[i];
    }

    /* Initialise a chunk buffer */
    caterva_ctx *ctx = src->ctx;
    int typesize = src->sc->typesize;
    int8_t *chunk = (int8_t *) ctx->alloc(src->csize * typesize);

    /* Calculate the constants out of the for  */
    size_t aux[CATERVA_MAXDIM];
    aux[7] = s_eshape[7] / s_pshape[7];
    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
        aux[i] = s_eshape[i] / s_pshape[i] * aux[i + 1];
    }

    /* Fill array from schunk (chunk by chunk) */
    size_t desp[CATERVA_MAXDIM], r[CATERVA_MAXDIM];
    for (size_t ci = 0; ci < src->esize / src->csize; ci++) {
        blosc2_schunk_decompress_chunk(src->sc, (int) ci, chunk, src->csize * typesize);

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
    return 0;
}

int _caterva_get_slice(caterva_array *src, void *dest, const size_t *start, const size_t *stop, const size_t *d_pshape) {
	uint8_t *bdest = dest;   // for allowing pointer arithmetic
    size_t s_shape[CATERVA_MAXDIM];
    size_t s_pshape[CATERVA_MAXDIM];
    size_t s_eshape[CATERVA_MAXDIM];
    size_t s_ndim = src->ndim;
    
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        s_shape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->shape[i];
        s_eshape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->eshape[i];
        s_pshape[(CATERVA_MAXDIM - s_ndim + i) % CATERVA_MAXDIM] = src->pshape[i];
    }
    
    /* Create chunk buffers */
    caterva_ctx *ctx = src->ctx;
    int typesize = src->sc->typesize;
    uint8_t *chunk = (uint8_t *) ctx->alloc(src->csize * typesize);

    size_t i_start[8], i_stop[8];
    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        i_start[i] = start[i] / s_pshape[i];
        i_stop[i] = (stop[i] - 1) / s_pshape[i];
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
                                        inc *= s_eshape[i] / s_pshape[i];
                                    }
                                    blosc2_schunk_decompress_chunk(src->sc, nchunk, chunk, src->csize * typesize);
                                    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
                                        if (ii[i] == (start[i] / s_pshape[i])) {
                                            c_start[i] = start[i] % s_pshape[i];
                                        } else {
                                            c_start[i] = 0;
                                        }
                                        if (ii[i] == stop[i] / s_pshape[i]) {
                                            c_stop[i] = stop[i] % s_pshape[i];
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
                                                                size_t chunk_pointer = 0;
                                                                size_t chunk_pointer_inc = 1;
                                                                for (int i = CATERVA_MAXDIM - 1; i >= 0; --i) {
                                                                    chunk_pointer += jj[i] * chunk_pointer_inc;
                                                                    chunk_pointer_inc *= s_pshape[i];
                                                                }
                                                                size_t buf_pointer = 0;
                                                                size_t buf_pointer_inc = 1;
                                                                for (int i = CATERVA_MAXDIM - 1; i >= 0; --i) {
                                                                    buf_pointer += (jj[i] + s_pshape[i] * ii[i] -
                                                                        start[i]) * buf_pointer_inc;
                                                                    buf_pointer_inc *= d_pshape[i];
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
    ctx->free(chunk);

    return 0;
}


int caterva_get_slice(caterva_array *dest, caterva_array *src, caterva_dims start, caterva_dims stop){

    if (start.ndim != stop.ndim) {
        return -1;
    }
    if (start.ndim != src->ndim) {
        return -1;
    }


    int typesize = src->sc->typesize;
    caterva_ctx *ctx = src->ctx;
    uint8_t *chunk = ctx->alloc(dest->csize * typesize);

    size_t shape_[CATERVA_MAXDIM];
    for (size_t i = 0; i < start.ndim; ++i) {
        shape_[i] = stop.dims[i] - start.dims[i];
    }
    for (int i = (int) start.ndim; i < CATERVA_MAXDIM; ++i) {
        shape_[i] = 1;
        start.dims[i] = 0;
    }
    
    caterva_dims shape = caterva_new_dims(shape_, start.ndim);
    _caterva_update_shape(dest, shape);

    size_t d_shape[CATERVA_MAXDIM];
    size_t d_pshape[CATERVA_MAXDIM];
    size_t d_eshape[CATERVA_MAXDIM];
    size_t d_start[CATERVA_MAXDIM];
    size_t d_stop[CATERVA_MAXDIM];
    size_t d_ndim = dest->ndim;

    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        d_shape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = dest->shape[i];
        d_eshape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = dest->eshape[i];
        d_pshape[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = dest->pshape[i];
        d_start[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = start.dims[i];
        d_stop[(CATERVA_MAXDIM - d_ndim + i) % CATERVA_MAXDIM] = stop.dims[i];
    }


    size_t ii[CATERVA_MAXDIM];
    for (ii[0] = d_start[0]; ii[0] < d_stop[0]; ii[0] += d_pshape[0]) {
        for (ii[1] = d_start[1]; ii[1] < d_stop[1]; ii[1] += d_pshape[1]) {
            for (ii[2] = d_start[2]; ii[2] < d_stop[2]; ii[2] += d_pshape[2]) {
                for (ii[3] = d_start[3]; ii[3] < d_stop[3]; ii[3] += d_pshape[3]) {
                    for (ii[4] = d_start[4]; ii[4] < d_stop[4]; ii[4] += d_pshape[4]) {
                        for (ii[5] = d_start[5]; ii[5] < d_stop[5]; ii[5] += d_pshape[5]) {
                            for (ii[6] = d_start[6]; ii[6] < d_stop[6]; ii[6] += d_pshape[6]) {
                                for (ii[7] = d_start[7]; ii[7] < d_stop[7]; ii[7] += d_pshape[7]) {
                                    memset(chunk, 0, dest->csize * typesize);
                                    size_t jj[CATERVA_MAXDIM];
                                    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
                                        if (ii[i] + d_pshape[i] > d_stop[i]) {
                                            jj[i] = d_stop[i];
                                        }
                                        else {
                                            jj[i] = ii[i] + d_pshape[i];
                                        }
                                    }

                                    _caterva_get_slice(src, chunk, ii, jj, d_pshape);

                                    blosc2_schunk_append_buffer(dest->sc, chunk, dest->csize * typesize);
                                }
                            }
                        }
                    }
                }
            }
        }   
    }
    caterva_squeeze(dest);
    return 0;
}

int caterva_repart(caterva_array *dest, caterva_array *src) {
    size_t start_[CATERVA_MAXDIM] = {0, 0, 0, 0, 0, 0, 0, 0};
    caterva_dims start = caterva_new_dims(start_, dest->ndim);
    size_t stop_[CATERVA_MAXDIM];
    for (int i = 0; i < dest->ndim; ++i) {
        stop_[i] = src->shape[i];
    }
    caterva_dims stop = caterva_new_dims(stop_, dest->ndim);
    caterva_get_slice(dest, src, start, stop);
    return 0;

}

int caterva_squeeze(caterva_array *src) {
    size_t nones = 0;
    size_t newshape_[CATERVA_MAXDIM];
    size_t newpshape_[CATERVA_MAXDIM];

    for (int i = 0; i < src->ndim; ++i) {
        if (src->shape[i] != 1) {
            newshape_[nones] = src->shape[i];
            newpshape_[nones] = src->pshape[i];
            nones += 1;
        }
    }

    for (int i = 0; i < CATERVA_MAXDIM; ++i) {
        if (i < nones){
            src->pshape[i] = newpshape_[i];
        } else {
            src->pshape[i] = 1;
        }
    }
    src->ndim = nones;

    caterva_dims newshape = caterva_new_dims(newshape_, nones);

    _caterva_update_shape(src, newshape);

    return 0;
}


caterva_dims caterva_get_shape(caterva_array *src){
    caterva_dims shape;
    for (int i = 0; i < src->ndim; ++i) {
        shape.dims[i] = src->shape[i];
    }
    shape.ndim = src->ndim;
    return shape;
}

caterva_dims caterva_get_pshape(caterva_array *src) {
    caterva_dims pshape;
    for (int i = 0; i < src->ndim; ++i) {
        pshape.dims[i] = src->pshape[i];
    }
    pshape.ndim = src->ndim;
    return pshape;
}