# Caterva

**Important:** All the features of Caterva have been included in the [C-Blosc2 NDim object](https://www.blosc.org/c-blosc2/c-blosc2.html). As a result, this project is now obsolete.

[![CI CMake](https://github.com/Blosc/caterva/actions/workflows/cmake.yml/badge.svg)](https://github.com/Blosc/caterva/actions/workflows/cmake.yml)
[![Documentation Status](https://readthedocs.org/projects/caterva/badge/?version=latest)](https://caterva.readthedocs.io/en/latest/?badge=latest)
[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-v2.0%20adopted-ff69b4.svg)](code_of_conduct.md)
## What it is

Caterva is a C library for handling multi-dimensional, compressed datasets in
an easy and convenient manner. It implements a thin metalayer on top of
[C-Blosc2](https://github.com/Blosc/c-blosc2) for specifying not only the
dimensionality of a dataset, but also the dimensionality of the chunks
inside the dataset. In addition, Caterva adds machinery for retrieving
arbitrary multi-dimensional slices (aka hyper-slices) out of the
multi-dimensional containers in the most efficient way. Hence, Caterva brings
the convenience of multi-dimensional and compressed containers to your
application very easily.

For more info, check out the
[Caterva documentation](https://caterva.readthedocs.io).
  
Here are slides of a gentle
[introductory talk to Caterva](https://blosc.github.io/caterva-scipy21).

## Credits
* Aleix Alcacer: original code creator
* Oscar Griñón: contributed the code for the second partitioning
* Marta Iborra: contributed the resizing operations for arrays
* Francesc Alted: direction, ideas and support code

This library has received the support from [a donation from Huawei to the Blosc project](https://www.blosc.org/posts/blosc-donation/).
Also, [ironArray SL](https://ironarray.io) has generously donated to the Blosc project, and Caterva in particular.
We are always grateful for donations that help the development of libraries under the umbrella
of Blosc.  Thanks!
