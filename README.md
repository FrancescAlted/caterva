# Caterva

[![Build Status](https://dev.azure.com/blosc/caterva/_apis/build/status/caterva?branchName=master)](https://dev.azure.com/blosc/caterva/_build/latest?definitionId=3&branchName=master)

## What it is

Caterva is a C library for handling multi-dimensional, compressed datasets in an easy and convenient manner.  It implements a thin metalayer on top of [C-Blosc2](https://github.com/Blosc/c-blosc2) for specifying not only the dimensionality of a dataset, but also the dimensionality of the chunks inside the dataset.  In addition, Caterva adds machinery for retrieving arbitrary multi-dimensional slices (aka hyper-slices) out of the multi-dimensional containers in the most efficient way.  Hence, Caterva brings the convenience of multi-dimensional and compressed containers to your application very easily.  For more info, check out the [Caterva documentation](https://caterva.readthedocs.io).
  
Here are slides of a gentle [introductory talk to Caterva](http://blosc.org/docs/Caterva-HDF5-Workshop.pdf).

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

A simple example of caterva usage are described in the file `examples/caterva_get_slice.c`. To execute it:

```
cd examples/
gcc -O -o caterva_get_slice.exe caterva_get_slice.c -lcaterva -lblosc
./caterva_get_slice.exe
```

## Python wrapper

Check the [cat4py](https://github.com/Blosc/cat4py) project for a Pythonic wrapper of Caterva.
