# Announcing Caterva 0.2.1: a simple multidimensional container for compressed data


## What is new?

In this release, both static and dynamic libraries are created by default now. If you want to
disable the creation of one of them, just set the cmake options `SHARED_LIB=OFF` or
`STATIC_LIB=OFF`. Also, a new `copy` parameter has been added to `caterva_from_file()`.

For more info, please see the release notes in:

https://github.com/Blosc/Caterva/blob/master/RELEASE_NOTES.md


What is it?
===========

Caterva is an open source C library and a format that allows to store large multidimensional,
chunked, compressed datasets.

Data can be stored either in-memory or on-disk, but the API to handle both versions is the same.
Compression is handled transparently for the user by adopting the Blosc2 library.

Download sources
================

The github repository is over here:

https://github.com/Blosc/Caterva

Caterva is distributed using the BSD license, see
[LICENSE](https://github.com/Blosc/Caterva/blob/master/LICENSE) for details.

Mailing list
============

There is an official Blosc mailing list where discussions about Caterva are welcome:

blosc@googlegroups.com
http://groups.google.es/group/blosc


Enjoy Data!
- The Blosc Development Team
