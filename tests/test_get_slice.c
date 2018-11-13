/*
 * Created by Aleix Alcacer
 */

#include "tests_common.h"

void assert_buf(double *exp, double *real, size_t size, double tol) {
    for (int i = 0; i < size; ++i) {
        LWTEST_ASSERT_ALMOST_EQUAL_DOUBLE(exp[i], real[i], tol);
    }
}

void print_buf(double *buf, size_t size) {
    for (int i = 0; i < size; ++i) {
        printf("%.f  ", buf[i]);
    }
}

void test_get_slice(caterva_array *src, size_t *start, size_t *stop){
    size_t buf_size = 1;
    for (int i = 0; i < src->ndim; ++i) {
        buf_size *= (stop[i] - start[i]);
    }

    double *buf = (double *) malloc(buf_size * src->sc->typesize);
    caterva_get_slice(src, buf, start, stop);
    print_buf(buf, buf_size);
    free(buf);
}

LWTEST_DATA(get_slice) {
    blosc2_cparams cp;
    blosc2_dparams dp;
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
    size_t shape[ndim] = {10, 10};
    size_t cshape[ndim] = {3, 2};
    size_t start[ndim] = {5, 3};
    size_t stop[ndim] = {9, 10};

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
    const size_t ndim = 3;
    size_t shape[ndim] = {10, 10, 10};
    size_t cshape[ndim] = {3, 5, 2};
    size_t start[ndim] = {3, 0, 3};
    size_t stop[ndim] = {6, 7, 10};

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