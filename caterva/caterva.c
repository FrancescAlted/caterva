/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#include "caterva.h"

caterva_array *caterva_new_array(blosc2_cparams cp, blosc2_dparams dp, blosc2_frame *fp, caterva_pparams pp) {
    /* Create a caterva_array buffer */
    caterva_array *carr = (caterva_array *) malloc(sizeof(caterva_array));

    /* Create a schunk */
    blosc2_schunk *sc = blosc2_new_schunk(cp, dp, fp);
    carr->sc = sc;
    carr->size = 1;
    carr->csize = 1;
    carr->esize = 1;
    carr->ndims = pp.ndims;

    for (unsigned int i = 0; i < CATERVA_MAXDIM; i++) {
        carr->shape[i] = pp.shape[i];
        carr->cshape[i] = pp.cshape[i];

        if (i >= CATERVA_MAXDIM - pp.ndims) {
            if (pp.shape[i] % pp.cshape[i] == 0) {
                carr->eshape[i] = pp.shape[i];
            } else {
                carr->eshape[i] = pp.shape[i] + pp.cshape[i] - pp.shape[i] % pp.cshape[i];
            }
        } else {
            carr->eshape[i] = 1;
        }
        carr->size *= carr->shape[i];
        carr->csize *= carr->cshape[i];
        carr->esize *= carr->eshape[i];
    }
    return carr;
}

int caterva_free_array(caterva_array *carr) {
    blosc2_free_schunk(carr->sc);
    free(carr);
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

    int typesize = dest->sc->typesize;
    int8_t *chunk = malloc(dest->csize * typesize);

    /* Calculate the constants out of the for  */
    size_t aux[CATERVA_MAXDIM];
    aux[7] = dest->eshape[7] / dest->cshape[7];
    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
        aux[i] = dest->eshape[i] / dest->cshape[i] * aux[i + 1];
    }

    /* Fill each chunk buffer */
    size_t desp[CATERVA_MAXDIM], r[CATERVA_MAXDIM];
    for (size_t ci = 0; ci < dest->esize / dest->csize; ci++) {
        memset(chunk, 0, dest->csize * typesize);

        /* Calculate the coord. of the chunk first element */
        desp[7] = ci % (dest->eshape[7] / dest->cshape[7]) * dest->cshape[7];
        for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
            desp[i] = ci % (aux[i]) / (aux[i + 1]) * dest->cshape[i];
        }

        /* Calculate if pad with 0 are needed in this chunk */
        for (int i = CATERVA_MAXDIM - 1; i >= 0; i--) {
            if (desp[i] + dest->cshape[i] > dest->shape[i]) {
                r[i] = dest->shape[i] - desp[i];
            } else {
                r[i] = dest->cshape[i];
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
                                    d_a = dest->cshape[7];

                                    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
                                        d_coord_f += ii[i] * d_a;
                                        d_a *= dest->cshape[i];
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

    free(chunk);
    return 0;
}

int caterva_to_buffer(caterva_array *src, void *dest) {
    int8_t *d_b = (int8_t *) dest;

    /* Initialise a chunk buffer */
    int typesize = src->sc->typesize;
    int8_t *chunk = (int8_t *) malloc(src->csize * typesize);

    /* Calculate the constants out of the for  */
    size_t aux[CATERVA_MAXDIM];
    aux[7] = src->eshape[7] / src->cshape[7];
    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
        aux[i] = src->eshape[i] / src->cshape[i] * aux[i + 1];
    }

    /* Fill array from schunk (chunk by chunk) */
    size_t desp[CATERVA_MAXDIM], r[CATERVA_MAXDIM];
    for (size_t ci = 0; ci < src->esize / src->csize; ci++) {
        blosc2_schunk_decompress_chunk(src->sc, (int)ci, chunk, src->csize * typesize);

        /* Calculate the coord. of the chunk first element in arr buffer */
        desp[7] = ci % aux[7] * src->cshape[7];
        for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
            desp[i] = ci % (aux[i]) / (aux[i + 1]) * src->cshape[i];
        }

        /* Calculate if pad with 0 are needed in this chunk */
        for (int i = CATERVA_MAXDIM - 1; i >= 0; i--) {
            if (desp[i] + src->cshape[i] > src->shape[i]) {
                r[i] = src->shape[i] - desp[i];
            } else {
                r[i] = src->cshape[i];
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
                                    s_a = src->cshape[7];

                                    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
                                        s_coord_f += ii[i] * s_a;
                                        s_a *= src->cshape[i];
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

    free(chunk);
    return 0;
}

int caterva_get_slice(caterva_array *src, caterva_array *dest, size_t *start, size_t *stop) {

    /* Create chunk buffers */
    int typesize = src->sc->typesize;
    int8_t *d_chunk = (int8_t *) malloc(dest->csize * typesize);
    int8_t *s_chunk = (int8_t *) malloc(src->csize * typesize);

    /* Calculate the constants out of the for (to avoid divisions into) */
    size_t s_aux[CATERVA_MAXDIM], d_aux[CATERVA_MAXDIM];
    s_aux[7] = 1;
    d_aux[7] = dest->eshape[7] / dest->cshape[7];
    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
        s_aux[i] = src->eshape[i + 1] / src->cshape[i + 1] * s_aux[i + 1];
        d_aux[i] = dest->eshape[i] / dest->cshape[i] * d_aux[i + 1];
    }

    size_t desp[CATERVA_MAXDIM], r[CATERVA_MAXDIM];
    size_t o_coord[CATERVA_MAXDIM], s_coord[CATERVA_MAXDIM], d_coord[CATERVA_MAXDIM];
    size_t s_ci;
    size_t s_coord_f, s_a, d_coord_f, d_a;

    /* Fill each chunk of dest */
    for (size_t d_ci = 0; d_ci < dest->esize / dest->csize; d_ci++) {
        memset(d_chunk, 0, dest->csize * typesize);

        /* Calculate the position of the dest_chunk first element in unpartitioned data */
        desp[7] = d_ci % (d_aux[7]) * dest->cshape[7] + start[7];
        for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
            desp[i] = d_ci % (d_aux[i]) / (d_aux[i + 1]) * dest->cshape[i] + start[i];
        }

        /* Calculate if pad with 0 are needed in this chunk */
        for (int i = CATERVA_MAXDIM - 1; i >= 0; i--) {
            if (desp[i] + dest->cshape[i] - start[i] > dest->shape[i]) {
                r[i] = dest->shape[i] - desp[i] + start[i];
            } else {
                r[i] = dest->cshape[i];
            }
        }

        /* Calculate the position of the desired element in dest_chunk */
        for (d_coord[0] = 0; d_coord[0] < r[0]; d_coord[0]++) {
            for (d_coord[1] = 0; d_coord[1] < r[1]; d_coord[1]++) {
                for (d_coord[2] = 0; d_coord[2] < r[2]; d_coord[2]++) {
                    for (d_coord[3] = 0; d_coord[3] < r[3]; d_coord[3]++) {
                        for (d_coord[4] = 0; d_coord[4] < r[4]; d_coord[4]++) {
                            for (d_coord[5] = 0; d_coord[5] < r[5]; d_coord[5]++) {
                                for (d_coord[6] = 0; d_coord[6] < r[6]; d_coord[6]++) {
                                    for (d_coord[7] = 0; d_coord[7] < r[7]; d_coord[7]++) {

                                        /* Calculate the position of the desired element in unpartitioned data */
                                        for (int i = CATERVA_MAXDIM - 1; i >= 0; i--) {
                                            o_coord[i] = d_coord[i] + desp[i];
                                        }

                                        /* Get src_chunk index and decompress it */
                                        s_ci = o_coord[7] / src->cshape[7];

                                        for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
                                            s_ci += (o_coord[i] / src->cshape[i]) * s_aux[i];
                                        }

                                        blosc2_schunk_decompress_chunk(src->sc, (int)s_ci, s_chunk,
                                                                       src->csize * typesize);

                                        /* Calculate the position of the desired element in src_chunk. */
                                        for (int i = CATERVA_MAXDIM - 1; i >= 0; i--) {
                                            s_coord[i] = o_coord[i] % src->cshape[i];
                                        }

                                        /* Flatten positions */
                                        s_coord_f = 0;
                                        s_a = 1;

                                        for (int i = CATERVA_MAXDIM - 1; i >= 0; i--) {
                                            s_coord_f += s_coord[i] * s_a;
                                            s_a *= src->cshape[i];
                                        }

                                        d_coord_f = 0;
                                        d_a = 1;

                                        for (int i = CATERVA_MAXDIM - 1; i >= 0; i--) {
                                            d_coord_f += d_coord[i] * d_a;
                                            d_a *= dest->cshape[i];
                                        }

                                        /* Copy the desired element from dest to src */
                                        memcpy(&d_chunk[d_coord_f * typesize],
                                               &s_chunk[s_coord_f * typesize], 1 * typesize);

                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        blosc2_schunk_append_buffer(dest->sc, d_chunk, dest->csize * typesize);

    }

    /* Free mallocs */
    free(s_chunk);
    free(d_chunk);
    return 0;
}

int caterva_equal_data(caterva_array *a, caterva_array *b){

    if (a->size != b->size) {
        return -1;
    }
    if (a->sc->typesize != b->sc->typesize) {
        return -1;
    }

    size_t size =  a->size;
    int typesize = a->sc->typesize;

    uint8_t *abuf = malloc(size * typesize);
    caterva_to_buffer(a, abuf);

    uint8_t *bbuf = malloc(size * typesize);
    caterva_to_buffer(b, bbuf);

    for (size_t i = 0; i < size * typesize; i++) {
        if (abuf[i] != bbuf[i]){
            return -1;
        }
    }
    free(abuf);
    free(bbuf);
    return 0;
}
