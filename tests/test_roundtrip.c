/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#include "test_common.h"

void test_roundtrip(caterva_array *src) {

    /* Create original data */
    double *bufsrc = (double *) malloc(src->size * sizeof(double));
    for (int i = 0; i < (int) src->size; i++) {
        bufsrc[i] = (double) i;
    }

    /* Fill empty caterva_array with original data */
    caterva_from_buffer(src, bufsrc);

    /* Fill dest array with caterva_array data */
    double *bufdest = (double *) malloc(src->size * sizeof(double));
    caterva_to_buffer(src, bufdest);

    /* Testing */
    for (size_t i = 0; i < src->size; i++) {
        LWTEST_ASSERT_ALMOST_EQUAL_DOUBLE(bufsrc[i], bufdest[i], 1e-15);
    }

    /* Free mallocs */
    free(bufsrc);
    free(bufdest);
}

LWTEST_DATA(roundtrip) {
    blosc2_cparams cp;
    blosc2_dparams dp;
    caterva_ctxt *ctxt;
};

LWTEST_SETUP(roundtrip) {
    data->cp = BLOSC_CPARAMS_DEFAULTS;
    data->cp.typesize = sizeof(double);
    data->dp = BLOSC_DPARAMS_DEFAULTS;
    data->ctxt = caterva_new_ctxt(NULL, NULL);
}

LWTEST_TEARDOWN(roundtrip) {
    data->ctxt->free(data->ctxt);
}

LWTEST_FIXTURE(roundtrip, 3_dim) {
    const size_t ndim = 3;
    size_t shape[ndim] = {4, 3, 3};
    size_t cshape[ndim] = {2, 2, 2};

    caterva_pparams src_pp = caterva_new_pparams(shape, cshape, ndim);
    caterva_array *src = caterva_new_array(data->cp, data->dp, NULL, src_pp, data->ctxt);

    test_roundtrip(src);
    caterva_free_array(src);
}

LWTEST_FIXTURE(roundtrip, 3_dim_2) {
    const size_t ndim = 3;
    size_t shape[ndim] = {134, 56, 204};
    size_t cshape[ndim] = {26, 17, 34};

    caterva_pparams src_pp = caterva_new_pparams(shape, cshape, ndim);
    caterva_array *src = caterva_new_array(data->cp, data->dp, NULL, src_pp, data->ctxt);

    test_roundtrip(src);
    caterva_free_array(src);
}

LWTEST_FIXTURE(roundtrip, 4_dim) {
    const size_t ndim = 4;
    size_t shape[ndim] = {4, 3, 8, 5};
    size_t cshape[ndim] = {2, 2, 3, 3};

    caterva_pparams src_pp = caterva_new_pparams(shape, cshape, ndim);
    caterva_array *src = caterva_new_array(data->cp, data->dp, NULL, src_pp, data->ctxt);

    test_roundtrip(src);
    caterva_free_array(src);
}

LWTEST_FIXTURE(roundtrip, 4_dim_2) {
    const size_t ndim = 4;
    size_t shape[ndim] = {78, 85, 34, 56};
    size_t cshape[ndim] = {13, 32, 18, 12};

    caterva_pparams src_pp = caterva_new_pparams(shape, cshape, ndim);
    caterva_array *src = caterva_new_array(data->cp, data->dp, NULL, src_pp, data->ctxt);

    test_roundtrip(src);
    caterva_free_array(src);
}

LWTEST_FIXTURE(roundtrip, 5_dim) {
    const size_t ndim = 5;
    size_t shape[ndim] = {4, 3, 8, 5, 10};
    size_t cshape[ndim] = {2, 2, 3, 3, 4};

    caterva_pparams src_pp = caterva_new_pparams(shape, cshape, ndim);
    caterva_array *src = caterva_new_array(data->cp, data->dp, NULL, src_pp, data->ctxt);

    test_roundtrip(src);
    caterva_free_array(src);
}

LWTEST_FIXTURE(roundtrip, 5_dim_2) {
    const size_t ndim = 5;
    size_t shape[ndim] = {35, 55, 24, 36, 12};
    size_t cshape[ndim] = {13, 32, 18, 12, 5};

    caterva_pparams src_pp = caterva_new_pparams(shape, cshape, ndim);
    caterva_array *src = caterva_new_array(data->cp, data->dp, NULL, src_pp, data->ctxt);

    test_roundtrip(src);
    caterva_free_array(src);
}

LWTEST_FIXTURE(roundtrip, 6_dim) {
    const size_t ndim = 6;
    size_t shape[ndim] = {4, 3, 8, 5, 10, 12};
    size_t cshape[ndim] = {2, 2, 3, 3, 4, 5};

    caterva_pparams src_pp = caterva_new_pparams(shape, cshape, ndim);
    caterva_array *src = caterva_new_array(data->cp, data->dp, NULL, src_pp, data->ctxt);

    test_roundtrip(src);
    caterva_free_array(src);
}

LWTEST_FIXTURE(roundtrip, 7_dim) {
    const size_t ndim = 7;
    size_t shape[ndim] = {12, 15, 24, 16, 12, 8, 7};
    size_t cshape[ndim] = {5, 7, 9, 8, 5, 3, 7};

    caterva_pparams src_pp = caterva_new_pparams(shape, cshape, ndim);
    caterva_array *src = caterva_new_array(data->cp, data->dp, NULL, src_pp, data->ctxt);

    test_roundtrip(src);
    caterva_free_array(src);
}

LWTEST_FIXTURE(roundtrip, 8_dim) {
    const size_t ndim = 8;
    size_t shape[ndim] = {4, 3, 8, 5, 10, 12, 6, 4};
    size_t cshape[ndim] = {3, 2, 3, 3, 4, 5, 4, 2};

    caterva_pparams src_pp = caterva_new_pparams(shape, cshape, ndim);
    caterva_array *src = caterva_new_array(data->cp, data->dp, NULL, src_pp, data->ctxt);

    test_roundtrip(src);
    caterva_free_array(src);
}