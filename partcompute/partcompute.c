#include "partcompute.h"

 caterva_array* partcompute_schunk_mul(caterva_array* a, caterva_array* b) {
        
    size_t P = a->cshape[0];
    
    size_t M = a->eshape[1];
    size_t K = a->eshape[0];
    size_t N = b->eshape[0];
    
    size_t p_size = P * P;
    size_t itemsize = a->sc->typesize;

    blosc2_cparams cp = BLOSC_CPARAMS_DEFAULTS;
    cp.typesize = sizeof(double);
    cp.filters[BLOSC_MAX_FILTERS - 1] = BLOSC_SHUFFLE;

    blosc2_dparams dp = BLOSC_DPARAMS_DEFAULTS;

    /* Create caterva_array src */

    caterva_pparams pp;

    pp.shape[0] = b->shape[0];
    pp.shape[1] = a->shape[1];

    pp.cshape[0] = P;
    pp.cshape[1] = P;

    for (int i = 2; i < CATERVA_MAXDIM; i++)
    {
        pp.shape[i] = 1;
        pp.cshape[i] = 1;
    }
    pp.dim = 2;

    caterva_array *c = caterva_new_array(cp, dp, pp);

        double * a_block = (double *) malloc(p_size * itemsize);
        double * b_block = (double *) malloc(p_size * itemsize);
        double * c_block;

        size_t a_i, b_i;
        size_t a_tam, b_tam;

        
        for(size_t m = 0; m < M / P; m++){
            for(size_t n = 0; n < N / P; n++){
                
                c_block = (double *) calloc(p_size, itemsize);
                
                
                for(size_t k = 0; k < K / P; k++){
                    a_i = (m * K / P + k);
                    b_i = (k * N / P + n);
                    
                    a_tam = blosc2_decompress_chunk(a->sc, a_i, a_block, p_size * itemsize);
                    b_tam = blosc2_decompress_chunk(b->sc, b_i, b_block, p_size * itemsize);
                    
                    
                    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, P, P, P,
                                1.0, a_block, P, b_block, P, 1.0, c_block, P); 
                    
                }
                
                blosc2_append_buffer(c->sc, p_size * itemsize, &c_block[0]);
            }
        }

        return c;

    }
