#include "caterva.h"

caterva_array* caterva_new_array(blosc2_cparams cp, blosc2_dparams dp, caterva_pparams pp)
{   
    /* Create a caterva_array buffer */

    caterva_array *carr = malloc(sizeof(caterva_array));

    /* Create a schunk */

    blosc2_schunk *sc = blosc2_new_schunk(cp, dp);
    carr->sc = sc;

    /* Fill all the caterva_array params */

    carr->size = 1;
    carr->csize = 1;
    carr->esize = 1;
    carr->dimensions = pp.dimensions;

    for(int i = 0; i < CATERVA_MAXDIM; i++)
    {
        carr->shape[i] = pp.shape[i];
        carr->cshape[i] = pp.cshape[i];
        
        if (i < pp.dimensions) {
            if (pp.shape[i] % pp.cshape[i] == 0){
                carr->eshape[i] = pp.shape[i];
            }
            else {
                carr->eshape[i] = pp.shape[i] + pp.cshape[i] - pp.shape[i] % pp.cshape[i];
            }
        }
        else {
            carr->eshape[i] = 1;
        }
        carr->size *= carr->shape[i];
        carr->csize *= carr->cshape[i];
        carr->esize *= carr->eshape[i];
    }
    return carr;
}

int caterva_free_array(caterva_array *carr)
{
    /* Free buffers used */

    blosc2_free_schunk(carr->sc);
    free(carr);
    return 0;
}

int caterva_schunk_fill_from_array(void *arr, caterva_array *carr)
{
    int8_t* arr_b = (int8_t *)arr;

    /* Define basic parameters */

    blosc2_schunk *sc = carr->sc;
    size_t *s = carr->shape;
    size_t *cs = carr->cshape;
    size_t *es = carr->eshape;
    size_t size = carr->size;
    size_t csize = carr->csize;
    size_t esize = carr->esize;
    size_t dimensions = carr->dimensions;
    int typesize = sc->typesize;

    /* Initialise a chunk buffer */

    int8_t *chunk = malloc(csize * typesize);

    /* Calculate the constants out of the for  */

    size_t aux[CATERVA_MAXDIM];

    aux[0] = es[0] / cs[0];

    for (int i = 1; i < CATERVA_MAXDIM; i++)
    {
        aux[i] = es[i] / cs[i] * aux[i - 1];
    }

    /* Fill each chunk buffer */

    size_t d[CATERVA_MAXDIM], r[CATERVA_MAXDIM];

    for (int ic = 0; ic < esize / csize; ic++)
    {
        memset(chunk, 0, csize * typesize);

        /* Calculate the coord. of the chunk first element */

        d[0] = ic % (es[0] / cs[0]) * cs[0];

        for (int i = 1; i < CATERVA_MAXDIM; i++)
        {
            d[i] = ic % (aux[i]) / (aux[i - 1]) * cs[i];
        }

        /* Calculate if pad with 0 are needed in this chunk */

        if (d[0] + cs[0] > s[0]) {
            r[0] = s[0] - d[0];
        }
        else {
            r[0] = cs[0];
        }

        for (int i = 1; i < CATERVA_MAXDIM; i++)
        {
            if (d[i] + cs[i] > s[i]) {
                r[i] = s[i] - d[i];
            }
            else {
                r[i] = cs[i];
            }
        }

        /* Copy each line of data from arr to buffer */

        size_t cpos, apos, aux;
        size_t ii[CATERVA_MAXDIM];

        for (ii[1] = 0; ii[1] < r[1]; ii[1]++)
        {
            for (ii[2] = 0; ii[2] < r[2]; ii[2]++)
            {
                for (ii[3] = 0; ii[3] < r[3]; ii[3]++)
                {
                    for (ii[4] = 0; ii[4] < r[4]; ii[4]++)
                    {
                        for (ii[5] = 0; ii[5] < r[5]; ii[5]++)
                        {
                            for (ii[6] = 0; ii[6] < r[6]; ii[6]++)
                            {
                                for (ii[7] = 0; ii[7] < r[7]; ii[7]++)
                                {
                                    cpos = 0;
                                    aux = cs[0];

                                    for (int i = 1; i < CATERVA_MAXDIM; i++)
                                    {
                                        cpos += ii[i] * aux;
                                        aux *= cs[i];
                                    }

                                    apos = d[0];
                                    aux = s[0];

                                    for (int i = 1; i < CATERVA_MAXDIM; i++)
                                    {
                                        apos += (d[i] + ii[i]) * aux;
                                        aux *= s[i];
                                    }
                                    memcpy(&chunk[cpos * typesize], &arr_b[apos * typesize], r[0] * typesize);
                                }
                            }
                        }
                    }
                }
            }
        }

        /* Append buffer to schunk */

        blosc2_append_buffer(sc, csize * typesize, chunk);
    }
    free(chunk);
    return 0;
}

int caterva_array_fill_from_schunk(caterva_array *s, void *d)
{
    int8_t *d_b = (int8_t *) d;

    /* Define basic parameters */

    int typesize = s->sc->typesize;

    /* Initialise a chunk buffer */

    int8_t *chunk = (int8_t*)malloc(s->csize * typesize);

    /* Calculate the constants out of the for  */

    size_t aux[CATERVA_MAXDIM];

    aux[0] = s->eshape[0] / s->cshape[0];

    for (int i = 1; i < CATERVA_MAXDIM; i++)
    {
        aux[i] = s->eshape[i] / s->cshape[i] * aux[i - 1];
    }

    /* Fill array from schunk (chunk by chunk) */

    size_t desp[CATERVA_MAXDIM], r[CATERVA_MAXDIM];

    for (int ci = 0; ci < s->esize / s->csize; ci++)
    {
        /* Decompress a chunk */

        blosc2_decompress_chunk(s->sc, ci, chunk, s->csize * typesize);
   
        /* Calculate the coord. of the chunk first element in arr buffer */

        desp[0] = ci % aux[0] * s->cshape[0];

        for (int i = 1; i < CATERVA_MAXDIM; i++)
        {
            desp[i] = ci % (aux[i]) / (aux[i - 1]) * s->cshape[i];
        }

        /* Calculate if pad with 0 are needed in this chunk */

        if (desp[0] + s->cshape[0] > s->shape[0]) {
            r[0] = s->shape[0] - desp[0];
        }
        else {
            r[0] = s->cshape[0];
        }
    
        for (int i = 1; i < CATERVA_MAXDIM; i++)
        {
            if (desp[i] + s->cshape[i] > s->shape[i]) {
                r[i] = s->shape[i] - desp[i];
            }
            else {
                r[i] = s->cshape[i];
            }
        }

        /* Copy each line of data from chunk to arr */

        size_t s_coord_f, d_coord_f, s_a, d_a;
        size_t ii[CATERVA_MAXDIM];

        for (ii[1] = 0; ii[1] < r[1]; ii[1]++)
        {
            for (ii[2] = 0; ii[2] < r[2]; ii[2]++)
            {
                for (ii[3] = 0; ii[3] < r[3]; ii[3]++)
                {
                    for (ii[4] = 0; ii[4] < r[4]; ii[4]++)
                    {
                        for (ii[5] = 0; ii[5] < r[5]; ii[5]++)
                        {
                            for (ii[6] = 0; ii[6] < r[6]; ii[6]++)
                            {
                                for (ii[7] = 0; ii[7] < r[7]; ii[7]++)
                                {
                                    s_coord_f = 0;
                                    s_a = s->cshape[0];

                                    for (int i = 1; i < CATERVA_MAXDIM; i++)
                                    {
                                        s_coord_f += ii[i] * s_a;
                                        s_a *= s->cshape[i];
                                    }

                                    d_coord_f = desp[0];
                                    d_a = s->shape[0];

                                    for (int i = 1; i < CATERVA_MAXDIM; i++)
                                    {
                                        d_coord_f += (desp[i] + ii[i]) * d_a;
                                        d_a *= s->shape[i];
                                    }
                                    memcpy(&d_b[d_coord_f * typesize], &chunk[s_coord_f * typesize], r[0] * typesize);
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

int caterva_get_slice(caterva_array *s, caterva_array *d, size_t start[], size_t stop[], size_t step[]) {
    
    /* Define basic parameters */

    int typesize = s->sc->typesize;

    /* Create chunk buffers */

    int8_t *d_chunk = (int8_t *) malloc(d->csize * typesize);
    int8_t *s_chunk = (int8_t *) malloc(s->csize * typesize);


    /* Calculate the constants out of the for (to avoid divisions into) */

    size_t s_aux[CATERVA_MAXDIM], d_aux[CATERVA_MAXDIM];

    s_aux[0] = 1;
    d_aux[0] = d->eshape[0] / d->cshape[0];

    for (int i = 1; i < CATERVA_MAXDIM; i++)
    {
        s_aux[i] = s->eshape[i] / s->cshape[i] * s_aux[i - 1];
        d_aux[i] = d->eshape[i] / d->cshape[i] * d_aux[i - 1];
    }

    size_t desp[CATERVA_MAXDIM], o_coord[CATERVA_MAXDIM], s_coord[CATERVA_MAXDIM], d_coord[CATERVA_MAXDIM];

    size_t s_ci, s_ci_b;
    
    size_t s_coord_f, s_a, d_coord_f, d_a;

    s_ci_b = -1;

    /* Fill each chunk of dest */

    for(size_t d_ci = 0; d_ci < d->esize / d->csize; d_ci++)
    {
    
        memset(d_chunk, 0, d->csize * typesize);

        /* Calculate the position of the dest_chunk first element in unpartitioned data */

        desp[0] = d_ci % (d_aux[0]) * d->cshape[0] + start[0];

        for (int i = 1; i < CATERVA_MAXDIM; i++)
        {
            desp[i] = d_ci % (d_aux[i]) / (d_aux[i - 1]) * d->cshape[i] + start[i];
        }
        
        /* Calculate the position of the desired element in dest_chunk */
        
        for(d_coord[7] = 0; d_coord[7] < d->cshape[7]; d_coord[7]++)
        {
            
            for(d_coord[6] = 0; d_coord[6] < d->cshape[6]; d_coord[6]++)
            {
                
                for(d_coord[5] = 0; d_coord[5] < d->cshape[5]; d_coord[5]++)
                {
                    
                    for(d_coord[4] = 0; d_coord[4] < d->cshape[4]; d_coord[4]++)
                    {
                        
                        for(d_coord[3] = 0; d_coord[3] < d->cshape[3]; d_coord[3]++)
                        {
                            
                            for(d_coord[2] = 0; d_coord[2] < d->cshape[2]; d_coord[2]++)
                            {
                                
                                for(d_coord[1] = 0; d_coord[1] < d->cshape[1]; d_coord[1]++)
                                {
                                    
                                    for(d_coord[0] = 0; d_coord[0] < d->cshape[0]; d_coord[0]++)
                                    {

                                        /* Calculate the position of the desired element in unpartitioned data */

                                        for(size_t i = 0; i < CATERVA_MAXDIM; i++)
                                        {
                                            o_coord[i] = d_coord[i] + desp[i];
                                        }

                                        /* Get src_chunk index and decompress it */

                                        s_ci = o_coord[0] / s->cshape[0];
                                        
                                        for(size_t i = 1; i < CATERVA_MAXDIM; i++)
                                        {
                                            s_ci += (o_coord[i] / s->cshape[i]) * s_aux[i];
                                        }

                                        if (s_ci != s_ci_b) {
                                            s_ci_b = s_ci;
                                            blosc2_decompress_chunk(s->sc, s_ci, s_chunk, s->csize * typesize);
                                        }
                                        
                                        /* Calculate the position of the desired element in src_chunk. */

                                        for(size_t i = 0; i < CATERVA_MAXDIM; i++)
                                        {
                                            s_coord[i] = o_coord[i] % s->cshape[i];
                                        }

                                        /* Flatten positions */

                                        s_coord_f = 0;
                                        s_a = 1;

                                        for(size_t i = 0; i < CATERVA_MAXDIM; i++)
                                        {
                                            s_coord_f += s_coord[i] * s_a;
                                            s_a *= s->cshape[i];
                                        }

                                        d_coord_f = 0;
                                        d_a = 1;
                                        
                                        for(size_t i = 0; i < CATERVA_MAXDIM; i++)
                                        {
                                            d_coord_f += d_coord[i] * d_a;
                                            d_a *= d->cshape[i];
                                        }

                                        /* Copy the desired element from dest to src */

                                        memcpy(&d_chunk[d_coord_f * typesize], &s_chunk[s_coord_f * typesize], 1 * typesize);

                                    }
                                    
                                }
                                
                            }
                            
                        }
                        
                    }
                    
                }
                
            }
                        
        }

        /* Append dest_chunk to dest array */

        blosc2_append_buffer(d->sc, d->csize * typesize, d_chunk);

    }

    /* Free mallocs */

    free(s_chunk);
    free(d_chunk);
    
    return 0;
}