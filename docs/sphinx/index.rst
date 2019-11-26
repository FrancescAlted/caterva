Caterva: a simple multidimensional container for compressed data
================================================================

..  image:: https://dev.azure.com/blosc/caterva/_apis/build/status/caterva?branchName=master
    :target: https://dev.azure.com/blosc/caterva/_build/latest?definitionId=3&branchName=master
    :alt: Build Status

..  image:: https://readthedocs.org/projects/caterva/badge/?version=latest
    :target: https://caterva.readthedocs.io/en/latest/?badge=latest
    :alt: Documentation Status

..  image:: https://img.shields.io/azure-devops/coverage/Blosc/Caterva/3
    :target: https://dev.azure.com/blosc/caterva/_build/latest?definitionId=5&branchName=master
    :alt: Coverage Status

Caterva is an open source C library that allows to store large multidimensional, chunked,
compressed datasets. Data can be stored either in-memory or on-disk, but the API to handle both
versions is the same.

In Caterva the compression is handled transparently for the user by leveraging the Blosc2 library.

Blosc is an extremely fast compressor specially designed for binary data. It uses the blocking
technique to reduce activity on the memory bus as much as possible. It also leverages SIMD
(SSE2, AVX2 for Intel, NEON for ARM, Altivec for Power) and multi-threading capabilities
present in multi-core processors so as to accelerate the compression/decompression process
to a maximum.

Being able to store in an in-memory data container does not mean that data cannot be persisted.
It is critical to find a way to store and retrieve data efficiently. Also, it is important to
adopt open formats for reducing the maintenance burden and facilitate its adoption more quickly.
Blosc2 brings such an efficient and open format for `persistency <https://github.com/Blosc/c-blosc2/blob/master/README_FRAME_FORMAT.rst>`__. This open format is used to create
persistent Caterva containers.

An aditional feature that introduces Blosc2 is the concept of metalayers. They are small metadata
for informing about the kind of data that is stored on a Blosc2 container. They are handy for
defining layers with different specs: data types, geo-spatial...

Caterva is created by specifying a metalayer on top of a Blosc2 container for storing
multidimensional information. This metalayer can be modified so that the shapes can be updated
(e.g. an array can grow or shrink).

Caterva’s main feature is to be able to extract all kind of slices out of high dimensional
datasets, efficiently. Resulting slices can be either Caterva containers or regular plain buffers.


..  toctree::
    :maxdepth: 2
    :caption: Contents

    installation
    api
    wrappers
