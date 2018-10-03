#include "caterva.h"

void schunk_fill_from_array(void *arr_b, caterva_array *carr)
{
    // int8_t* arr_b = (int8_t *)arr;

    /* Define basic parameters */

    caterva_pparams *pp = carr->pp;
    blosc2_schunk *sc = carr->sc;

    size_t *s = pp->shape;
    size_t *cs = pp->cshape;
    size_t *es = pp->eshape;
    size_t dimensions = pp->dimensions;
    int typesize = sc->typesize;

    size_t size = 1;
    size_t csize = 1;
    size_t esize = 1;

    for (int i = 0; i < MAXDIM; i++)
    {
        size *= s[i];
        csize *= cs[i];
        esize *= es[i];
    }

    /* Initialise a chunk buffer */

    int8_t *chunk = malloc(csize * typesize);

    /* Calculate the constants out of the for  */

    size_t aux[MAXDIM];

    aux[0] = es[0] / cs[0];

    for (int i = 1; i < MAXDIM; i++)
    {
        aux[i] = es[i] / cs[i] * aux[i - 1];
    }

    /* Fill each chunk buffer */

    size_t d[MAXDIM], r[MAXDIM];

    for (int ic = 0; ic < esize / csize; ic++)
    {
        memset(chunk, 0, csize * typesize);

        /* Calculate the coord. of the chunk first element */

        d[0] = ic % (es[0] / cs[0]) * cs[0];

        for (int i = 1; i < MAXDIM; i++)
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

        for (int i = 1; i < MAXDIM; i++)
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
        size_t ii[MAXDIM];

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

                                    for (int i = 1; i < MAXDIM; i++)
                                    {
                                        cpos += ii[i] * aux;
                                        aux *= cs[i];
                                    }

                                    apos = d[0];
                                    aux = s[0];

                                    for (int i = 1; i < MAXDIM; i++)
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
}

void array_fill_from_schunk(caterva_array *carr, void *arr)
{
    int8_t* arr_b = (int8_t *)arr;

    /* Define basic parameters */

    caterva_pparams *pp = carr->pp;
    blosc2_schunk *sc = carr->sc;
    
    size_t *s = pp->shape;
    size_t *cs = pp->cshape;
    size_t *es = pp->eshape;
    size_t dimensions = pp->dimensions;
    int typesize = sc->typesize;

    size_t size = 1;
    size_t csize = 1;
    size_t esize = 1;

    for (int i = 0; i < MAXDIM; i++)
    {
        size *= s[i];
        csize *= cs[i];
        esize *= es[i];
    }

    /* Initialise a chunk buffer */

    int8_t *chunk = (int8_t*)malloc(csize * typesize);
    double *chunk_d = (double*)chunk;

    /* Calculate the constants out of the for  */

    size_t aux[MAXDIM];

    aux[0] = es[0] / cs[0];

    for (int i = 1; i < MAXDIM; i++)
    {
        aux[i] = es[i] / cs[i] * aux[i - 1];
    }

    /* Fill array from schunk (chunk by chunk) */

    size_t d[MAXDIM], r[MAXDIM];

    for (int ic = 0; ic < esize / csize; ic++)
    {
        /* Decompress a chunk */

        blosc2_decompress_chunk(sc, ic, chunk, csize * typesize);
   
        /* Calculate the coord. of the chunk first element in arr buffer */

        d[0] = ic % (es[0] / cs[0]) * cs[0];

        for (int i = 1; i < MAXDIM; i++)
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
    
        for (int i = 1; i < MAXDIM; i++)
        {
            if (d[i] + cs[i] > s[i]) {
                r[i] = s[i] - d[i];
            }
            else {
                r[i] = cs[i];
            }
        }

        /* Copy each line of data from chunk to arr */

        size_t cpos, apos, aux;
        size_t ii[MAXDIM];

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

                                    for (int i = 1; i < MAXDIM; i++)
                                    {
                                        cpos += ii[i] * aux;
                                        aux *= cs[i];
                                    }

                                    apos = d[0];
                                    aux = s[0];

                                    for (int i = 1; i < MAXDIM; i++)
                                    {
                                        apos += (d[i] + ii[i]) * aux;
                                        aux *= s[i];
                                    }
                                    memcpy(&arr_b[apos * typesize], &chunk[cpos * typesize], r[0] * typesize);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}