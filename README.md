# Caterva

[![Build Status](https://dev.azure.com/blosc/caterva/_apis/build/status/caterva?branchName=master)](https://dev.azure.com/blosc/caterva/_build/latest?definitionId=3&branchName=master)
![Azure DevOps coverage](https://img.shields.io/azure-devops/coverage/Blosc/Caterva/3)

## What it is

Caterva is both a C library and a format layer on top of Blosc2.  Here are slides of a gentle [introductory talk to Caterva](http://blosc.org/docs/Caterva-HDF5-Workshop.pdf).


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
