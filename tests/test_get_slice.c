/*
 * Created by Aleix Alcacer
 */

#include "tests_common.h"

void test_get_slice(caterva_array *src, size_t *start, size_t *stop){
    double *buf = (double *) malloc(src->size * src->sc->typesize);
    caterva_get_slice(src, buf, start, stop);
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
    size_t shape[ndim] = {6, 4};
    size_t cshape[ndim] = {3, 2};
    size_t start[ndim] = {1, 1};
    size_t stop[ndim] = {2, 3};

    caterva_pparams pp = caterva_new_pparams(shape, cshape, ndim);
    caterva_array *src = caterva_new_array(data->cp, data->dp, NULL, pp);

    double *buf = (double *) malloc(src->size * src->sc->typesize);
    for (int i = 0; i < src->size; ++i) {
        buf[i] = i;
    }
    caterva_from_buffer(src, buf);

    test_get_slice(src, start, stop);
}
