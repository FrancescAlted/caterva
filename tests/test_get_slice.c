/*
 * Created by Aleix Alcacer
 */

#include "tests_common.h"

void print_matrix(size_t m, size_t n, double *buf){
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            printf("%5.2f", buf[i*n + j]);
        }
    printf("\n");
    }
}
void test_get_slice(caterva_array *src, size_t *start, size_t *stop){
    size_t buf_size = 1;
    for (int i = 0; i < src->ndim; ++i) {
        buf_size *= (stop[i] - start[i]);
    }

    double *buf = (double *) malloc(buf_size * src->sc->typesize);
    caterva_get_slice(src, buf, start, stop);
    //print_matrix(stop[0] - start[0], stop[1] - start[1], buf);
}

LWTEST_DATA(get_slice) {
    blosc2_cparams cp;
    blosc2_dparams dp;
    double *buf;
};

LWTEST_SETUP(get_slice){
    data->cp = BLOSC_CPARAMS_DEFAULTS;
    data->cp.typesize = sizeof(double);
    data->dp = BLOSC_DPARAMS_DEFAULTS;
}

LWTEST_TEARDOWN(get_slice){

}

LWTEST_FIXTURE(get_slice, testing) {
    const size_t ndim = 2;
    size_t shape[ndim] = {6, 4};
    size_t cshape[ndim] = {3, 2};
    size_t start[ndim] = {1, 1};
    size_t stop[ndim] = {4, 4};

    caterva_pparams pp = caterva_new_pparams(shape, cshape, ndim);
    caterva_array *src = caterva_new_array(data->cp, data->dp, NULL, pp);

    double *buf = (double *) malloc(src->size * src->sc->typesize);
    for (int i = 0; i < src->size; ++i) {
        buf[i] = i;
    }
    caterva_from_buffer(src, buf);

    test_get_slice(src, start, stop);

    caterva_free_array(src);
    free(buf);
}


LWTEST_FIXTURE(get_slice, testing_2) {
    const size_t ndim = 5;
    size_t shape[ndim] = {60, 38, 45, 32, 54};
    size_t cshape[ndim] = {11, 13, 23, 12, 21};
    size_t start[ndim] = {16, 3, 2, 6, 7};
    size_t stop[ndim] = {45, 31, 33, 18, 23};

    caterva_pparams pp = caterva_new_pparams(shape, cshape, ndim);
    caterva_array *src = caterva_new_array(data->cp, data->dp, NULL, pp);

    double *buf = (double *) malloc(src->size * src->sc->typesize);
    for (int i = 0; i < src->size; ++i) {
        buf[i] = i;
    }
    caterva_from_buffer(src, buf);

    test_get_slice(src, start, stop);

    caterva_free_array(src);
    free(buf);
}