#include "tests_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <blosc.h>

#define MAXDIM 8

int tests_run = 0;

int convert_to_array(char *line, size_t *shape)
{
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

void get_field(char *line, size_t *shape, size_t *cshape, int* dimensions)
{
    char *shape_str;
    char *cshape_str;
    char *tok;
    char *tmp = line;

    tok = strtok(tmp, ";");
    shape_str = strdup(tok);
    tok = strtok(NULL, ";");
    cshape_str = strdup(tok);
    tok = strtok(NULL, ";");
    *dimensions = atoi(tok);

    convert_to_array(shape_str, shape);
    convert_to_array(cshape_str, cshape);
}

char* test_roundtrip(size_t shape[], size_t cshape[], int dimensions)
{
    size_t eshape[MAXDIM];
    size_t size = 1;
    size_t csize = 1;
    size_t esize = 1;

    for (int i = 0; i < MAXDIM; i++)
    {
        if (i < dimensions) {
            if (shape[i] % cshape[i] == 0){
                eshape[i] = shape[i];
            }
            else {
                eshape[i] = (size_t)shape[i] + cshape[i] - shape[i] % cshape[i];
            }
        }
        else {
            eshape[i] = 1;
        }
        
        size *= shape[i];
        csize *= cshape[i];
        esize *= eshape[i];
    }

    double *arr = (double *)malloc(size * sizeof(double));

    for (int i = 0; i < size; i++)
    {
        arr[i] = (double)i;
    }

    /* Create dparams, cparams and pparams */

    blosc2_cparams cp = BLOSC_CPARAMS_DEFAULTS;
    cp.typesize = sizeof(double);
    cp.filters[BLOSC_MAX_FILTERS - 1] = BLOSC_SHUFFLE;

    blosc2_dparams dp = BLOSC_DPARAMS_DEFAULTS;

    caterva_pparams pp;

    for (int i = 0; i < MAXDIM; i++)
    {
        pp.shape[i] = shape[i];
        pp.cshape[i] = cshape[i];
        pp.eshape[i] = eshape[i];
    }

    pp.dimensions = dimensions;

    /* Create new schunk */

    blosc2_schunk *sc;
    sc = blosc2_new_schunk(cp, dp);

    /* Create a caterva array */

    caterva_array carr;
    carr.pp = &pp;
    carr.sc = sc;


    /* Fill empty schunk with arr data */

    schunk_fill_from_array(arr, &carr);

    /* Fill new array with schunk data */

    double *arr_dest = (double *)malloc(size * sizeof(double));

    array_fill_from_schunk(&carr, arr_dest);

    for(size_t i = 0; i < size; i++)
    {
        mu_assert("ERROR. Original and resulting arrays are not equal!", arr[i] == arr_dest[i]);
    }

    free(arr);
    free(arr_dest);
    return 0;
}

static char* all_tests(size_t shape[], size_t cshape[], int* dimensions) {

    FILE *stream = fopen("test_schunk_fill_from_array_roundtrip.csv", "r");

    char line[1024];

    mu_assert("ERROR al abrir el fichero csv", stream != NULL);

    fgets(line, 1024, stream);

    while (fgets(line, 1024, stream))
    {
        char *tmp = line;
        get_field(tmp, shape, cshape, dimensions);
        mu_run_test(test_roundtrip(shape, cshape, *dimensions));
    }

    return 0;
}

int main()
{
    setbuf(stdout, NULL);
    
    size_t shape[MAXDIM];
    size_t cshape[MAXDIM];
    int dimensions;

    char* result = all_tests(shape, cshape, &dimensions);

    if (result != 0) {
        printf(" (%s)", result);
    }
    else {
        printf(" ALL TESTS PASSED");
    }
    printf("\tTests run: %d\n", tests_run);

    return 0;
}
