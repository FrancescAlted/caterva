#include "tests_common.h"

int main(int argc, char const *argv[])
{
    /* Define parameters values */

    size_t src_shape[CATERVA_MAXDIM] = {128, 128, 128, 1, 1, 1, 1, 1};
    size_t src_cshape[CATERVA_MAXDIM] = {16, 16, 16, 1, 1, 1, 1, 1};
    size_t src_dim = 3;

    /* Define start, stop and step values */

    size_t start[CATERVA_MAXDIM] = {15, 4, 80, 0, 0, 0, 0, 0};
    size_t stop[CATERVA_MAXDIM] = {20, 8, 120, 1, 1, 1, 1, 1};
    size_t step[CATERVA_MAXDIM] = {1, 1, 1, 1, 1, 1, 1, 1}; /* Not working */
    size_t dest_cshape[CATERVA_MAXDIM] = {3, 2, 12, 1, 1, 1, 1, 1};
    size_t dest_dim = 3;

    /* Create dparams and cparams */

    blosc2_cparams cp = BLOSC_CPARAMS_DEFAULTS;
    cp.typesize = sizeof(double);
    cp.filters[BLOSC_MAX_FILTERS - 1] = BLOSC_SHUFFLE;

    blosc2_dparams dp = BLOSC_DPARAMS_DEFAULTS;

    /* Create caterva_array src */

    caterva_pparams src_pp;
    for (int i = 0; i < CATERVA_MAXDIM; i++)
    {
        src_pp.shape[i] = src_shape[i];
        src_pp.cshape[i] = src_cshape[i];
    }
    src_pp.dim = src_dim;

    caterva_array *src = caterva_new_array(cp, dp, src_pp);

    /* Create caterva_array dest */

    caterva_pparams dest_pp;
    for (int i = 0; i < CATERVA_MAXDIM; i++)
    {
        dest_pp.shape[i] = stop[i] - start[i];
        dest_pp.cshape[i] = dest_cshape[i];
    }
    dest_pp.dim = dest_dim;

    caterva_array *dest = caterva_new_array(cp, dp, dest_pp);

    /* Create original data */

    double *arr = (double *)malloc(src->size * sizeof(double));
    for (int i = 0; i < src->size; i++)
    {
        arr[i] = (double)i;
    }
    
    /* Fill src caterva_array with original data */

    caterva_schunk_fill_from_array(arr, src);

    /* Obtain a subset of data from src caterva_array */

    caterva_get_slice(src, dest, start, stop, step);

    /* Create array dest */

    double *arr_dest = (double *)malloc(dest->size * sizeof(double));

    /* Fill array dest from dest caterva array */

    caterva_array_fill_from_schunk(dest, arr_dest);

    /* Testing */

    
    for(size_t i = 0; i < dest->size; i++)
    {
        printf("%.2f\n", arr_dest[i]);
    }
    
    caterva_free_array(src);
    caterva_free_array(dest);
    return 0;
}