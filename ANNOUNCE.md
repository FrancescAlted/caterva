# Announcing Caterva 0.4.0: a simple multidimensional container for compressed data


## What is new?

An API renaming has been done to adapt some Caterva variable names to those
used by the general community.

Also, a new level of multi-dimensionality has been introduced. As a result,
unlike other libraries, Caterva supports two levels of multi-dimensional
chunking (chunks and blocks).

For more info, please see the release notes in:

https://github.com/Blosc/Caterva/releases


## What is it?

Caterva is an open source C library and a format that allows to store large
multidimensional, chunked, compressed datasets.

Data can be stored either in-memory or on-disk, but the API to handle both
versions is the same. Compression is handled transparently for the user by
adopting the Blosc2 library.

## Download sources

The github repository is over here:

https://github.com/Blosc/Caterva

Caterva is distributed using the BSD license, see
[LICENSE](https://github.com/Blosc/Caterva/blob/master/LICENSE) for details.

## Mailing list

There is an official Blosc mailing list where discussions about Caterva are
welcome:

blosc@googlegroups.com

http://groups.google.es/group/blosc


Enjoy Data!
- The Blosc Development Team
