from cffi import FFI
ffibuilder = FFI()

# cdef() expects a string listing the C types, functions and
# globals needed from Python. The string follows the C syntax.
ffibuilder.cdef("""
enum {
      BLOSC_MAX_FILTERS = 5, /* Maximum number of filters in the filter pipeline */
      CATERVA_MAXDIM = 8,
    };
    
    typedef struct blosc2_context_s blosc2_context;

    typedef struct {
      int32_t nthreads; /* the number of threads to use internally (1) */
      void* schunk; /* the associated schunk, if any (NULL) */
    } blosc2_dparams;

    typedef struct {
      int compcode; /* the compressor codec */
      int clevel; /* the compression level (5) */
      int use_dict; /* use dicts or not when compressing (only for ZSTD) */
      size_t typesize; /* the type size (8) */
      uint32_t nthreads; /* the number of threads to use internally (1) */
      size_t blocksize; /* the requested size of the compressed blocks (0; meaning automatic) */
      void* schunk; /* the associated schunk, if any (NULL) */
      uint8_t filters[BLOSC_MAX_FILTERS]; /* the (sequence of) filters */
      uint8_t filters_meta[BLOSC_MAX_FILTERS]; /* metadata for filters */
    } blosc2_cparams;

    typedef struct {
      uint8_t version;
      uint8_t flags1;
      uint8_t flags2;
      uint8_t flags3;
      uint8_t compcode;  // starts at 4 bytes
      /* The default compressor.  Each chunk can override this. */
      uint8_t clevel;  // starts at 6 bytes
      /* The compression level and other compress params */
      uint32_t typesize;
      /* the type size */
      int32_t blocksize;
      /* the requested size of the compressed blocks (0; meaning automatic) */
      uint32_t chunksize;   // starts at 8 bytes
      /* Size of each chunk.  0 if not a fixed chunksize. */
      uint8_t filters[BLOSC_MAX_FILTERS];  // starts at 12 bytes
      /* The (sequence of) filters.  8-bit per filter. */
      uint8_t filters_meta[BLOSC_MAX_FILTERS];
      /* Metadata for filters. 8-bit per meta-slot. */
      int64_t nchunks;  // starts at 28 bytes
      /* Number of chunks in super-chunk */
      int64_t nbytes;  // starts at 36 bytes
      /* data size + metadata size + header size (uncompressed) */
      int64_t cbytes;  // starts at 44 bytes
      /* data size + metadata size + header size (compressed) */
      uint8_t* filters_chunk;  // starts at 52 bytes
      /* Pointer to chunk hosting filter-related data */
      uint8_t* codec_chunk;
      /* Pointer to chunk hosting codec-related data */
      uint8_t* metadata_chunk;
      /* Pointer to schunk metadata */
      uint8_t* userdata_chunk;
      /* Pointer to user-defined data */
      uint8_t** data;
      /* Pointer to chunk data pointers */
      //uint8_t* ctx;
      /* Context for the thread holder.  NULL if not acquired. */
      blosc2_context* cctx;
      blosc2_context* dctx;
      /* Contexts for compression and decompression */
      uint8_t* reserved;
      /* Reserved for the future. */
    } blosc2_schunk;


typedef struct
{
    size_t shape[CATERVA_MAXDIM]; /* the shape of original data */
    size_t cshape[CATERVA_MAXDIM]; /* the shape of each chunk */
    size_t dim; /* data dimensions */
} caterva_pparams;

typedef struct
{
    blosc2_schunk* sc;
    size_t shape[CATERVA_MAXDIM]; /* shape of original data */
    size_t cshape[CATERVA_MAXDIM]; /* shape of each chunk */
    size_t eshape[CATERVA_MAXDIM]; /* shape of schunk */
    size_t size; /* size of original data */
    size_t csize; /* size of each chunnk */
    size_t esize; /* shape of schunk */
    size_t dim; /* data dimensions */
} caterva_array;



caterva_array* caterva_new_array(blosc2_cparams cp, blosc2_dparams dp, caterva_pparams pp);

int caterva_free_array(caterva_array *carr);

int caterva_schunk_fill_from_array(void *arr, caterva_array *carr);

int caterva_array_fill_from_schunk(caterva_array *carr, void *arr);

int caterva_get_slice(caterva_array *src, caterva_array *dest, size_t start[], size_t stop[], size_t step[]);

caterva_array* partcompute_schunk_mul(caterva_array* a, caterva_array* b);

""")

# This describes the extension module "_pi_cffi" to produce.
ffibuilder.set_source("demo_",
"""
     #include "partcompute.h"
     #include "caterva.h" 
     # define CATERVA_MAXDIM 8

""",  library_dirs=["/opt/intel/mkl/lib"],
      include_dirs=["/opt/intel/mkl/include"],
      libraries=['partcompute', 'caterva', 'blas'])   # library name, for the linker

if __name__ == "__main__":
    ffibuilder.compile(verbose=True)