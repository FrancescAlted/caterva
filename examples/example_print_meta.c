/*
 * Copyright (C) 2019-present Blosc Development team <blosc@blosc.org>
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
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

int main() {
    print_meta(NULL);   // here you must introduce the frame urlpath
    return 0;
}
