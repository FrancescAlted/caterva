from demo_ import ffi, lib

def caterva_create_cparams(compcode, clevel, use_dict, typesize, nthreads, blocksize, schunk, filters, filters_meta):
    cp = ffi.new("blosc2_cparams*")
    cp.compcode = compcode
    cp.clevel = clevel
    cp.use_dict = use_dict
    cp.typesize = typesize
    cp.nthreads = nthreads
    cp.blocksize = blocksize
    cp.schunk = ffi.NULL if schunk is None else schunk
    cp.filters = filters
    cp.filters_meta = filters_meta
    return cp[0]


def caterva_create_dparams(nthreads, schunk):
    dp = ffi.new("blosc2_dparams*")
    dp.nthreads = nthreads
    dp.schunk = ffi.NULL if schunk is None else schunk
    return dp[0]

def caterva_create_pparams(shape, cshape, dim):
    pp = ffi.new("caterva_pparams*")
    pp.shape = shape
    pp.cshape = cshape
    pp.dim = dim
    return pp[0]

def caterva_new_array(cp, dp, pp):
    return lib.caterva_new_array(cp, dp, pp)

def caterva_free_array(s):
    return lib.caterva_free_array(s)

def caterva_schunk_fill_from_array(s, d):
    s = ffi.from_buffer(s)
    return lib.caterva_schunk_fill_from_array(s, d)

def caterva_array_fill_from_schunk(s, d):
    d = ffi.from_buffer(d)
    return lib.caterva_array_fill_from_schunk(s, d)

def caterva_get_slice(s, d, start, stop, step):
    return lib.caterva_get_slice(s, d, start, stop, step)

def  partcompute_schunk_mul(a, b):
    return lib.partcompute_schunk_mul(a, b)