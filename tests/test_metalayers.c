/*
 * Copyright (c) 2018 Francesc Alted, Aleix Alcacer.
 * Copyright (C) 2019-present Blosc Development team <blosc@blosc.org>
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 */

#include "test_common.h"


CUTEST_TEST_DATA(metalayers) {
    caterva_ctx_t *ctx;
};


CUTEST_TEST_SETUP(metalayers) {
    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    cfg.nthreads = 2;
    cfg.compcodec = BLOSC_BLOSCLZ;
    caterva_ctx_new(&cfg, &data->ctx);

    // Add parametrizations
    CUTEST_PARAMETRIZE(itemsize, uint8_t, CUTEST_DATA(1, 2, 4, 8));
    CUTEST_PARAMETRIZE(shapes, _test_shapes, CUTEST_DATA(
            {0, {0}, {0}, {0}}, // 0-dim
            {1, {10}, {7}, {2}}, // 1-idim
            {2, {100, 100}, {20, 20}, {10, 10}},
    ));
    CUTEST_PARAMETRIZE(sequencial, bool, CUTEST_DATA(true, false));
}


CUTEST_TEST_TEST(metalayers) {
    CUTEST_GET_PARAMETER(shapes, _test_shapes);
    CUTEST_GET_PARAMETER(itemsize, uint8_t);
    CUTEST_GET_PARAMETER(sequencial, bool);

    char *urlpath = "test_metalayers.caterva";
    caterva_remove(data->ctx, urlpath);
    caterva_params_t params;
    params.itemsize = itemsize;
    params.ndim = shapes.ndim;
    for (int i = 0; i < params.ndim; ++i) {
        params.shape[i] = shapes.shape[i];
    }

    caterva_storage_t storage = {0};
    storage.backend = CATERVA_STORAGE_BLOSC;
    storage.properties.blosc.urlpath = urlpath;
    storage.properties.blosc.sequencial = sequencial;
    for (int i = 0; i < params.ndim; ++i) {
        storage.properties.blosc.chunkshape[i] = shapes.chunkshape[i];
        storage.properties.blosc.blockshape[i] = shapes.blockshape[i];
    }

    /* Create original data */
    size_t buffersize = itemsize;
    for (int i = 0; i < params.ndim; ++i) {
        buffersize *= (size_t) params.shape[i];
    }


    uint8_t *buffer = malloc(buffersize);
    CUTEST_ASSERT("Buffer filled incorrectly", fill_buf(buffer, itemsize, buffersize / itemsize));

    /* Create caterva_array_t with original data */
    caterva_array_t *src;
    CATERVA_TEST_ASSERT(caterva_from_buffer(data->ctx, buffer, buffersize, &params, &storage,
                                            &src));

    caterva_metalayer_t vlmeta1;

    uint64_t sdata1 = 56;
    vlmeta1.name = "vlmeta1";
    vlmeta1.sdata = (uint8_t *) &sdata1;
    vlmeta1.size = sizeof(sdata1);

    CATERVA_TEST_ASSERT(caterva_vlmeta_add(data->ctx, src, &vlmeta1));

    bool exists = false;
    CATERVA_ERROR(caterva_vlmeta_exists(data->ctx, src, "vlmeta2", &exists));
    CATERVA_TEST_ASSERT(exists == false);
    CATERVA_ERROR(caterva_vlmeta_exists(data->ctx, src, vlmeta1.name, &exists));
    CATERVA_TEST_ASSERT(exists == true);

    caterva_metalayer_t vlmeta2;
    CATERVA_ERROR(caterva_vlmeta_get(data->ctx, src, vlmeta1.name, &vlmeta2));
    CUTEST_ASSERT("Contents are not equals",
                  *((uint64_t *) vlmeta1.sdata) == *((uint64_t *) vlmeta2.sdata));
    CUTEST_ASSERT("Sizes are not equals", vlmeta1.size == vlmeta2.size);
    free(vlmeta2.name);
    free(vlmeta2.sdata);

    float sdata11 = 4.5f;
    vlmeta1.sdata = (uint8_t *) &sdata11;
    vlmeta1.size = sizeof(sdata11);

    CATERVA_TEST_ASSERT(caterva_vlmeta_update(data->ctx, src, &vlmeta1));

    caterva_metalayer_t vlmeta3;
    CATERVA_ERROR(caterva_vlmeta_get(data->ctx, src, vlmeta1.name, &vlmeta3));
    CUTEST_ASSERT("Contents are not equals", *((float *) vlmeta1.sdata) == *((float *) vlmeta3.sdata));
    CUTEST_ASSERT("Sizes are not equals", vlmeta1.size == vlmeta3.size);
    free(vlmeta3.name);
    free(vlmeta3.sdata);

    vlmeta2.name = "vlmeta2";
    vlmeta2.sdata = (uint8_t *) &sdata1;
    vlmeta2.size = sizeof(sdata1);
    CATERVA_ERROR(caterva_vlmeta_add(data->ctx, src, &vlmeta2));

    caterva_array_t *src2;
    caterva_open(data->ctx, urlpath, &src2);

    CATERVA_ERROR(caterva_vlmeta_get(data->ctx, src2, vlmeta2.name, &vlmeta3));
    CUTEST_ASSERT("Contents are not equals", *((uint64_t *) vlmeta2.sdata) == *((uint64_t *) vlmeta3.sdata));
    CUTEST_ASSERT("Sizes are not equals", vlmeta2.size == vlmeta3.size);
    free(vlmeta3.name);
    free(vlmeta3.sdata);

    /* Free mallocs */
    free(buffer);
    CATERVA_TEST_ASSERT(caterva_free(data->ctx, &src));
    CATERVA_TEST_ASSERT(caterva_free(data->ctx, &src2));
    caterva_remove(data->ctx, urlpath);
    return 0;
}


CUTEST_TEST_TEARDOWN(metalayers) {
    caterva_ctx_free(&data->ctx);
}

int main() {
    CUTEST_TEST_RUN(metalayers);
}
