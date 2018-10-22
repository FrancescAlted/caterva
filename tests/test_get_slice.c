#include "tests_common.h"

int tests_run = 0;

int convert_to_array_d(char *line, double *shape)
{
    /* Convert string to an array */

    char *tok;
    tok = strtok(line, "-");
    int i = 0;
    while (tok != NULL)
    {
        sscanf(tok, "%lf", &shape[i]);
        tok = strtok(NULL, "-");
        i++;
    }
    return 0;
}
int convert_to_array(char *line, size_t *shape)
{
    /* Convert string to an array */

    char *tok;
    tok = strtok(line, "-");
    int i = 0;
    while (tok != NULL)
    {
        sscanf(tok, "%lu", &shape[i]);
        tok = strtok(NULL, "-");
        i++;
    }
    return 0;
}

int get_fields(char *line, size_t *src_shape, size_t *src_cshape, int* src_dim, size_t *start, size_t *stop,
                size_t *step, size_t *dest_cshape, int* dest_dim, double **res)
{
    /* Get the fields of a csv line */

    char *src_shape_str;
    char *src_cshape_str;
    char *src_dim_str;
    char *start_str;
    char *stop_str;
    char *step_str;
    char *dest_cshape_str;
    char* dest_dim_str;
    char* res_str;
    char *tok;
    char *tmp = line;

    tok = strtok(tmp, ";");
    src_shape_str = strdup(tok);
    tok = strtok(NULL, ";");
    src_cshape_str = strdup(tok);
    tok = strtok(NULL, ";");
    src_dim_str = strdup(tok);


    tok = strtok(NULL, ";");
    start_str = strdup(tok);
    tok = strtok(NULL, ";");
    stop_str = strdup(tok);
    tok = strtok(NULL, ";");
    step_str = strdup(tok);
    tok = strtok(NULL, ";");
    dest_cshape_str = strdup(tok);
    tok = strtok(NULL, ";");
    dest_dim_str = strdup(tok);
    tok = strtok(NULL, ";");
    res_str = strdup(tok);

    convert_to_array(src_shape_str, src_shape);
    convert_to_array(src_cshape_str, src_cshape);
    *src_dim = atoi(src_dim_str);
    convert_to_array(start_str, start);
    convert_to_array(stop_str, stop);
    convert_to_array(step_str, step);
    convert_to_array(dest_cshape_str, dest_cshape);
    *dest_dim = atoi(dest_dim_str);
    
    size_t dest_size = 1;
    for(int i = 0; i < CATERVA_MAXDIM; i++)
    {
        dest_size *= start[i]-stop[i];
    }
    *res = malloc(dest_size * sizeof(double));
    
    convert_to_array_d(res_str, *res);
    return 0;
}

char* test_roundtrip(size_t src_shape[], size_t src_cshape[], int src_dim, size_t start[], size_t stop[], 
                     size_t step[], size_t dest_cshape[], int dest_dim, double *res)
{
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
        printf("%lf - %lf\n",  res[i], arr_dest[i]);
        mu_assert("ERROR. Original and resulting arrays are not equal!", res[i] == arr_dest[i]);

    }
    
    caterva_free_array(src);
    caterva_free_array(dest);
    free(arr);
    free(res);
    free(arr_dest);

    return 0;
}

static char* all_tests(size_t src_shape[], size_t src_cshape[], int* src_dim, size_t start[], size_t stop[], 
                       size_t step[], size_t dest_cshape[], int *dest_dim, double **res) {

    /* Read csv file (generated via notebook generating_results_for_get_slice.ipynb) */

    FILE *stream = fopen("test_get_slice.csv", "r");
    mu_assert("ERROR al abrir el fichero csv", stream != NULL);

    /* Run a test for each line of csv file */

    char line[32768];
    fgets(line, 32768, stream);
    while (fgets(line, 32768, stream))
    {
        char *tmp = line;
        get_fields(tmp, src_shape, src_cshape, src_dim, start, stop, step, dest_cshape, dest_dim, res);
        mu_run_test(test_roundtrip(src_shape, src_cshape, *src_dim, start, stop, step, dest_cshape, *dest_dim, *res));
    }
    return 0;
}


int main()
{
    /* Set stream buffer */

    setbuf(stdout, NULL);
    
    /* Define data needed for run a test */

    /* Define src values */

    size_t src_shape[CATERVA_MAXDIM];
    size_t src_cshape[CATERVA_MAXDIM];
    int src_dim;

    /* Define start, stop and step values */

    size_t start[CATERVA_MAXDIM];
    size_t stop[CATERVA_MAXDIM];
    size_t step[CATERVA_MAXDIM]; /* Not working */

    /* Define dest values */
    size_t dest_cshape[CATERVA_MAXDIM];
    int dest_dim;
    double *res;

    /* Print test result */

    char* result = all_tests(src_shape, src_cshape, &src_dim, start, stop, step, dest_cshape, &dest_dim, &res);

    if (result != 0) {
        printf(" (%s)", result);
    }
    else {
        printf(" ALL TESTS PASSED");
    }
    printf("\tTests run: %d\n", tests_run);

    return 0;
}