# Caterva

[![Build Status](https://dev.azure.com/blosc/caterva/_apis/build/status/caterva?branchName=master)](https://dev.azure.com/blosc/caterva/_build/latest?definitionId=3&branchName=master)

## What it is

Caterva is an open source C library that allows to store large multidimensional, chunked,
compressed datasets. Data can be stored either in-memory or on-disk, but the API to handle both
versions is the same. 

### Blosc2

In Caterva the compression is handled transparently for the user by leveraging the Blosc2 library.

Blosc is an extremely fast compressor specially designed for binary data. It uses the blocking
technique to reduce activity on the memory bus as much as possible. It also leverages SIMD
(SSE2, AVX2 for Intel, NEON for ARM, Altivec for Power) and multi-threading capabilities
present in multi-core processors so as to accelerate the compression/decompression process
to a maximum.

Being able to store in an in-memory data container does not mean that data cannot be persisted.
It is critical to find a way to store and retrieve data efficiently. Also, it is important to
adopt open formats for reducing the maintenance burden and facilitate its adoption more quickly.
Blosc2 brings such an efficient and open format for persistency.

## Installation

```
mkdir build
cd build
```
```
cmake -DCMAKE_INSTALL_PREFIX='your_install_directory' ..
```
```
cmake --build .
ctest
cmake --build . --target install
```

## Examples

A simple example of caterva usage are described in the file `examples/simple.c`. To execute it:

```
cd examples/
gcc -O -o simple.exe simple.c -lcaterva -lblosc
./simple.exe
```

## Python wrapper

Check the [cat4py](https://github.com/Blosc/cat4py) project for a Pythonic wrapper of Caterva.
