/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#include "tests_common.h"

int tests_run = 0;

int convert_to_array_d(char *line, double *shape) {
    /* Convert string to an array */
    char *tok;
    tok = strtok(line, "-");
    int i = 0;
    while (tok != NULL) {
        shape[i] = strtod(tok, NULL);
        tok = strtok(NULL, "-");
        i++;
    }
    return 0;
}

int convert_to_array(char *line, size_t *shape) {
    /* Convert string to an array */
    char *tok;
    tok = strtok(line, "-");
    int i = 0;
    while (tok != NULL) {
        shape[i] = strtoul(tok, NULL, 10);
        tok = strtok(NULL, "-");
        i++;
    }
    return 0;
}

/* Get the fields of a csv line */
int get_fields(char *line, size_t *src_shape, size_t *src_cshape, size_t *src_dim, size_t *start,
               size_t *stop, size_t *dest_cshape, size_t *dest_dim, double **res) {
    char *src_shape_str;
    char *src_cshape_str;
    char *src_dim_str;
    char *start_str;
    char *stop_str;
    char *dest_cshape_str;
    char *dest_dim_str;
    char *res_str;
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
    dest_cshape_str = strdup(tok);
    tok = strtok(NULL, ";");
    dest_dim_str = strdup(tok);
    tok = strtok(NULL, ";");
    res_str = strdup(tok);

    convert_to_array(src_shape_str, src_shape);
    convert_to_array(src_cshape_str, src_cshape);
    *src_dim = (size_t)strtol(src_dim_str, NULL, 10);
    convert_to_array(start_str, start);
    convert_to_array(stop_str, stop);
    convert_to_array(dest_cshape_str, dest_cshape);
    *dest_dim = (size_t)strtol(dest_dim_str, NULL, 10);

    size_t dest_size = 1;
    for (int i = 0; i < CATERVA_MAXDIM; i++) {
        dest_size *= start[i] - stop[i];
    }
    *res = malloc(dest_size * sizeof(double));

    convert_to_array_d(res_str, *res);
    return 0;
}

char *test_roundtrip(const size_t *src_shape, const size_t *src_cshape, size_t src_dim,
                     size_t *start, size_t *stop, const size_t *dest_cshape,
                     size_t dest_dim, double *res) {
    /* Create dparams and cparams */
    blosc2_cparams cp = BLOSC_CPARAMS_DEFAULTS;
    cp.typesize = sizeof(double);
    cp.filters[BLOSC_MAX_FILTERS - 1] = BLOSC_SHUFFLE;

    blosc2_dparams dp = BLOSC_DPARAMS_DEFAULTS;

    /* Create caterva_array src */
    caterva_pparams src_pp;
    for (int i = 0; i < CATERVA_MAXDIM; i++) {
        src_pp.shape[i] = src_shape[i];
        src_pp.cshape[i] = src_cshape[i];
    }
    src_pp.ndims = src_dim;

    caterva_array *src = caterva_new_array(cp, dp, NULL, src_pp);

    /* Create caterva_array dest */
    caterva_pparams dest_pp;
    for (int i = 0; i < CATERVA_MAXDIM; i++) {
        dest_pp.shape[i] = stop[i] - start[i];
        dest_pp.cshape[i] = dest_cshape[i];
    }
    dest_pp.ndims = dest_dim;

    caterva_array *dest = caterva_new_array(cp, dp, NULL, dest_pp);

    /* Create original data */
    double *arr = (double *) malloc(src->size * sizeof(double));
    for (unsigned int i = 0; i < src->size; i++) {
        arr[i] = (double) i;
    }

    caterva_from_buffer(src, arr);
    caterva_get_slice(src, dest, start, stop);
    double *arr_dest = (double *) malloc(dest->size * sizeof(double));
    caterva_to_buffer(dest, arr_dest);

    for (size_t i = 0; i < dest->size; i++) {
        mu_assert("ERROR. Original and resulting arrays are not equal!", res[i] == arr_dest[i]);
    }

    caterva_free_array(src);
    caterva_free_array(dest);
    free(arr);
    free(res);
    free(arr_dest);

    return 0;
}

static char *all_tests(char *filename, size_t *src_shape, size_t *src_cshape, size_t *src_dim,
                       size_t *start, size_t *stop, size_t *dest_cshape, size_t *dest_dim,
                       double **res) {

    /* Read csv file (generated via notebook generating_results_for_get_slice.ipynb) */
    FILE *stream = fopen(filename, "r");
    mu_assert("ERROR al abrir el fichero csv", stream != NULL);

    /* Run a test for each line of csv file */
    char line[32768];
    fgets(line, 32768, stream);
    while (fgets(line, 32768, stream)) {
        char *tmp = line;
        get_fields(tmp, src_shape, src_cshape, src_dim, start, stop, dest_cshape, dest_dim, res);
        mu_run_test(test_roundtrip(src_shape, src_cshape, *src_dim, start, stop, dest_cshape,
                                   *dest_dim, *res));
    }
    return 0;
}

int main(int argc, char **argv) {
    /* Set stream buffer */
    setbuf(stdout, NULL);

    /* Define data needed for run a test */

    /* Set src values */
    char *filename = NULL;
    if (argc > 1) {
        filename = argv[1];
    } else {
        printf("Please, pass the source dims in a CSV file");
    }
    size_t src_shape[CATERVA_MAXDIM];
    size_t src_cshape[CATERVA_MAXDIM];
    size_t src_dim;

    /* Define start, stop and step values */
    size_t start[CATERVA_MAXDIM];
    size_t stop[CATERVA_MAXDIM];

    /* Define dest values */
    size_t dest_cshape[CATERVA_MAXDIM];
    size_t dest_dim;
    double *res;

    char *result = all_tests(filename, src_shape, src_cshape, &src_dim, start, stop,
                             dest_cshape, &dest_dim, &res);

    if (result != 0) {
        printf(" (%s)", result);
    } else {
        printf(" ALL TESTS PASSED");
    }
    printf("\tTests run: %d\n", tests_run);

    return result != 0;
}