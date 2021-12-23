/*
 * Copyright (C) 2019-present Blosc Development team <blosc@blosc.org>
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 * Test program demonstrating use of the Blosc codec from C code.
 * To compile this program:
 *
 * $ gcc -O example_print_meta.c -o example_print_meta -lblosc2
 * $ <urlpath>
 *
 * Caterva metalayer parameters:
 * Ndim:       _
 * Shape:      _, _, _
 * Chunkshape: _, _, _
 * Blockshape: _, _, _
*/

# include <caterva.h>

int print_meta(char *urlpath) {

    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    caterva_ctx_t *ctx;
    caterva_ctx_new(&cfg, &ctx);
    caterva_array_t *arr;
    CATERVA_ERROR(caterva_open(ctx, urlpath, &arr));
    caterva_print_meta(arr);
    caterva_free(ctx, &arr);
    caterva_ctx_free(&ctx);

    return 0;
}

int main(int argc, char *argv[]) {
    char* urlpath;                      // you can build frames using example_frame_generator.c
    for( int i = 0; i < argc; ++i ) {
        urlpath = argv[i];
        print_meta(urlpath);
    }
    return 0;
}
