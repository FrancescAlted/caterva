#ifndef HEADER_FILE
#define HEADER_FILE

#include <stdio.h>
#include <blosc.h>

#define MAXDIM 8

typedef struct
{
    size_t shape[MAXDIM];  /* the shape of original data */
    size_t cshape[MAXDIM]; /* the shape of each chunk */
    size_t dimensions;     /* data dimensions */
} caterva_pparams;

typedef struct
{
    blosc2_schunk* sc;
    size_t shape[MAXDIM];  /* the shape of original data */
    size_t cshape[MAXDIM]; /* the shape of each chunk */
    size_t eshape[MAXDIM]; /* the shape of schunk */
    size_t size;
    size_t csize;
    size_t esize;
    size_t dimensions;     /* data dimensions */
} caterva_array;


void schunk_fill_from_array(void *arr_b, caterva_array *carr);

void array_fill_from_schunk(caterva_array *carr, void *arr);

#endif
