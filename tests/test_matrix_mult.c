#include "tests_common.h"

int main(int argc, char const *argv[])
{
     blosc_timestamp_t last, current;

    /* Define parameters values */

    size_t a_shape[CATERVA_MAXDIM] = {8192, 8192, 1, 1, 1, 1, 1, 1};
    size_t a_cshape[CATERVA_MAXDIM] = {1024, 1024, 1, 1, 1, 1, 1, 1};
    size_t a_dim = 2;

    size_t b_shape[CATERVA_MAXDIM] = {8192, 8192, 1, 1, 1, 1, 1, 1};
    size_t b_cshape[CATERVA_MAXDIM] = {1024, 1024, 1, 1, 1, 1, 1, 1};
    size_t b_dim = 2;

    /* Create dparams and cparams */

    blosc2_cparams cp = BLOSC_CPARAMS_DEFAULTS;
    cp.typesize = sizeof(double);
    cp.filters[BLOSC_MAX_FILTERS - 1] = BLOSC_SHUFFLE;

    blosc2_dparams dp = BLOSC_DPARAMS_DEFAULTS;

    /* Create caterva_array a */

    caterva_pparams a_pp;
    for (int i = 0; i < CATERVA_MAXDIM; i++)
    {
        a_pp.shape[i] = a_shape[i];
        a_pp.cshape[i] = a_cshape[i];
    }
    a_pp.dim = a_dim;

    caterva_array *a = caterva_new_array(cp, dp, a_pp);

    /* Create caterva_array b */

    caterva_pparams b_pp;
    for (int i = 0; i < CATERVA_MAXDIM; i++)
    {
        b_pp.shape[i] = b_shape[i];
        b_pp.cshape[i] = b_cshape[i];
    }
    b_pp.dim = b_dim;

    caterva_array *b = caterva_new_array(cp, dp, b_pp);


    /* Create a data */

    double *a_arr = (double *)malloc(a->size * sizeof(double));
    for (size_t i = 0; i < a->size; i++)
    {
        a_arr[i] = (double)i;
    }
    
    /* Fill src caterva_array with original data */

    caterva_schunk_fill_from_array(a_arr, a);

    /* Create b data */

    double *b_arr = (double *)malloc(b->size * sizeof(double));
    for (size_t i = 0; i < b->size; i++)
    {
        b_arr[i] = (double)i;
    }
    
    /* Fill src caterva_array with original data */

    caterva_schunk_fill_from_array(b_arr, b);

    /* Multiply matrix using mkl */

    blosc_set_timestamp(&last);

    caterva_array *c = partcompute_schunk_mul(a, b);

    blosc_set_timestamp(&current);

    double ttotal = blosc_elapsed_secs(last, current);

    double operations = (2 * a_shape[0] - 1) * a_shape[1] * b_shape[0];

    double gflops = operations / (ttotal * 10e9);

    printf("Gflops: %.2f\n", gflops);

    /* Create array c */

    double *c_arr = (double *)malloc(c->size * sizeof(double));

    /* Fill array dest from dest caterva array */

    caterva_array_fill_from_schunk(c, c_arr);

    /* Testing */

    /*
    size_t cont = 0;

    for(size_t i = 0; i < c->shape[0]; i++)
    {
        for(size_t j = 0; j < c->shape[1]; j++)
        {
            printf("%8.f", c_arr[cont++]);
        }
        printf("\n");

    }
    */

    caterva_free_array(a);
    caterva_free_array(b);
    caterva_free_array(c);

    free(a_arr);
    free(b_arr);
    free(c_arr);

    return 0;
}