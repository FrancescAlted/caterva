# Caterva

[![Build Status](https://dev.azure.com/blosc/caterva/_apis/build/status/caterva?branchName=master)](https://dev.azure.com/blosc/caterva/_build/latest?definitionId=3&branchName=master)
[![Documentation Status](https://readthedocs.org/projects/caterva/badge/?version=latest)](https://caterva.readthedocs.io/en/latest/?badge=latest)


## What it is

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
Blosc2 brings such an efficient and open format for persistency. This open format is used to create
persistent Caterva containers.

A aditional feature thet introduces Blosc2 is the concept of metalayers. They are small metadata
for informing about the kind of data that is stored on a Blosc2 container. They are handy for
defining layers with different specs: data types, geo-spatial... 

Caterva is created by specifying a metalayer on top of a Blosc2 container for storing
multidimensional information. This metalayer can be modified so that the shapes can be updated
(e.g. an array can grow or shrink). (See more about the Caterva metalayer format in
[CATERVA_METALAYER](CATERVA_NETALAYER.md).

Caterva’s main feature is to be able to extract all kind of slices out of high dimesional
datasets, efficiently. Resulting slices can be either Caterva containers or regular plain buffers.


## Building Caterva with CMake

Caterva can be built, tested and installed using CMake. The following procedure describes a
typical CMake build. In order to install Caterva, you need to have the library c-blosc2 builded
 or installed.

1. Create the build directory inside the sources and move into it:
  
    ```
    cd c-blosc2-sources
    mkdir build/
    cd build/
    ```

2. Now run CMake configuration and, if necessary, specify the directory where the Blosc build is
and the directory where the Blosc headers are:
   ```
    cmake -DBLOSC_DIR='blosc_build_dir' -DBLOSC_INCLUDE='blosc_headers_dir' ..
    ```
 
3. Build and test Caterva:

    ```
    cmake --build .
   ctest
   ```
   
4. If desired, install Caterva:
    ```
    cmake --build . --target install
    ```
   

## Wrappers

- [cat4py](https://github.com/Blosc/cat4py): a Pythonic wrapper of Caterva.
