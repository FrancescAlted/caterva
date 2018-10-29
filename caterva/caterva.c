/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#include "caterva.h"

caterva_array *caterva_new_array(blosc2_cparams cp, blosc2_dparams dp, caterva_pparams pp) {
    /* Create a caterva_array buffer */
    caterva_array *carr = (caterva_array *) malloc(sizeof(caterva_array));

    /* Create a schunk */
    const blosc2_frame *fr = &BLOSC_EMPTY_FRAME;
    blosc2_schunk *sc = blosc2_new_schunk(cp, dp, fr);
    carr->sc = sc;
    carr->size = 1;
    carr->csize = 1;
    carr->esize = 1;
    carr->dim = pp.dim;

    for (int i = 0; i < CATERVA_MAXDIM; i++) {
        carr->shape[i] = pp.shape[i];
        carr->cshape[i] = pp.cshape[i];

        if (i >= CATERVA_MAXDIM - pp.dim) {
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

int caterva_schunk_fill_from_array(void *s, caterva_array *d) {
    int8_t *s_b = (int8_t *) s;

    int typesize = d->sc->typesize;
    int8_t *chunk = malloc(d->csize * typesize);

    /* Calculate the constants out of the for  */
    size_t aux[CATERVA_MAXDIM];
    aux[7] = d->eshape[7] / d->cshape[7];
    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
        aux[i] = d->eshape[i] / d->cshape[i] * aux[i + 1];
    }

    /* Fill each chunk buffer */
    size_t desp[CATERVA_MAXDIM], r[CATERVA_MAXDIM];
    for (int ci = 0; ci < d->esize / d->csize; ci++) {
        memset(chunk, 0, d->csize * typesize);

        /* Calculate the coord. of the chunk first element */
        desp[7] = ci % (d->eshape[7] / d->cshape[7]) * d->cshape[7];
        for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
            desp[i] = ci % (aux[i]) / (aux[i + 1]) * d->cshape[i];
        }

        /* Calculate if pad with 0 are needed in this chunk */
        for (int i = CATERVA_MAXDIM - 1; i >= 0; i--) {
            if (desp[i] + d->cshape[i] > d->shape[i]) {
                r[i] = d->shape[i] - desp[i];
            } else {
                r[i] = d->cshape[i];
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
                                    d_a = d->cshape[7];

                                    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
                                        d_coord_f += ii[i] * d_a;
                                        d_a *= d->cshape[i];
                                    }

                                    s_coord_f = desp[7];
                                    s_a = d->shape[7];

                                    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
                                        s_coord_f += (desp[i] + ii[i]) * s_a;
                                        s_a *= d->shape[i];
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
        blosc2_schunk_append_buffer(d->sc, chunk, d->csize * typesize);
    }

    free(chunk);
    return 0;
}

int caterva_array_fill_from_schunk(caterva_array *s, void *d) {
    int8_t *d_b = (int8_t *) d;

    /* Initialise a chunk buffer */
    int typesize = s->sc->typesize;
    int8_t *chunk = (int8_t *) malloc(s->csize * typesize);

    /* Calculate the constants out of the for  */
    size_t aux[CATERVA_MAXDIM];
    aux[7] = s->eshape[7] / s->cshape[7];
    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
        aux[i] = s->eshape[i] / s->cshape[i] * aux[i + 1];
    }

    /* Fill array from schunk (chunk by chunk) */
    size_t desp[CATERVA_MAXDIM], r[CATERVA_MAXDIM];
    for (int ci = 0; ci < s->esize / s->csize; ci++) {
        blosc2_schunk_decompress_chunk(s->sc, ci, chunk, s->csize * typesize);

        /* Calculate the coord. of the chunk first element in arr buffer */
        desp[7] = ci % aux[7] * s->cshape[7];
        for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
            desp[i] = ci % (aux[i]) / (aux[i + 1]) * s->cshape[i];
        }

        /* Calculate if pad with 0 are needed in this chunk */
        for (int i = CATERVA_MAXDIM - 1; i >= 0; i--) {
            if (desp[i] + s->cshape[i] > s->shape[i]) {
                r[i] = s->shape[i] - desp[i];
            } else {
                r[i] = s->cshape[i];
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
                                    s_a = s->cshape[7];

                                    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
                                        s_coord_f += ii[i] * s_a;
                                        s_a *= s->cshape[i];
                                    }

                                    d_coord_f = desp[7];
                                    d_a = s->shape[7];

                                    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
                                        d_coord_f += (desp[i] + ii[i]) * d_a;
                                        d_a *= s->shape[i];
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

int caterva_get_slice(caterva_array *s, caterva_array *d, size_t start[], size_t stop[],
                      size_t step[]) {

    /* Create chunk buffers */
    int typesize = s->sc->typesize;
    int8_t *d_chunk = (int8_t *) malloc(d->csize * typesize);
    int8_t *s_chunk = (int8_t *) malloc(s->csize * typesize);

    /* Calculate the constants out of the for (to avoid divisions into) */
    size_t s_aux[CATERVA_MAXDIM], d_aux[CATERVA_MAXDIM];
    s_aux[7] = 1;
    d_aux[7] = d->eshape[7] / d->cshape[7];
    for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
        s_aux[i] = s->eshape[i + 1] / s->cshape[i + 1] * s_aux[i + 1];
        d_aux[i] = d->eshape[i] / d->cshape[i] * d_aux[i + 1];
    }

    size_t desp[CATERVA_MAXDIM], r[CATERVA_MAXDIM];
    size_t o_coord[CATERVA_MAXDIM], s_coord[CATERVA_MAXDIM], d_coord[CATERVA_MAXDIM];
    size_t s_ci;
    size_t s_coord_f, s_a, d_coord_f, d_a;

    /* Fill each chunk of dest */
    for (size_t d_ci = 0; d_ci < d->esize / d->csize; d_ci++) {
        memset(d_chunk, 0, d->csize * typesize);

        /* Calculate the position of the dest_chunk first element in unpartitioned data */
        desp[7] = d_ci % (d_aux[7]) * d->cshape[7] + start[7];
        for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
            desp[i] = d_ci % (d_aux[i]) / (d_aux[i + 1]) * d->cshape[i] + start[i];
        }

        /* Calculate if pad with 0 are needed in this chunk */
        for (int i = CATERVA_MAXDIM - 1; i >= 0; i--) {
            if (desp[i] + d->cshape[i] - start[i] > d->shape[i]) {
                r[i] = d->shape[i] - desp[i] + start[i];
            } else {
                r[i] = d->cshape[i];
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
                                        s_ci = o_coord[7] / s->cshape[7];

                                        for (int i = CATERVA_MAXDIM - 2; i >= 0; i--) {
                                            s_ci += (o_coord[i] / s->cshape[i]) * s_aux[i];
                                        }

                                        blosc2_schunk_decompress_chunk(s->sc, s_ci, s_chunk,
                                                                       s->csize * typesize);

                                        /* Calculate the position of the desired element in src_chunk. */
                                        for (int i = CATERVA_MAXDIM - 1; i >= 0; i--) {
                                            s_coord[i] = o_coord[i] % s->cshape[i];
                                        }

                                        /* Flatten positions */
                                        s_coord_f = 0;
                                        s_a = 1;

                                        for (int i = CATERVA_MAXDIM - 1; i >= 0; i--) {
                                            s_coord_f += s_coord[i] * s_a;
                                            s_a *= s->cshape[i];
                                        }

                                        d_coord_f = 0;
                                        d_a = 1;

                                        for (int i = CATERVA_MAXDIM - 1; i >= 0; i--) {
                                            d_coord_f += d_coord[i] * d_a;
                                            d_a *= d->cshape[i];
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

        blosc2_schunk_append_buffer(d->sc, d_chunk, d->csize * typesize);

    }

    /* Free mallocs */
    free(s_chunk);
    free(d_chunk);
    return 0;
}
