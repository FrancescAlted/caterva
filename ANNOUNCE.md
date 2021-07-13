# Announcing Caterva 0.5.0: a simple multidimensional container for compressed data


## What is new?

A redesign of the caterva guts and a code refactorization have been performed
in order to simplify the code. This includes API renaming.

Also a set_slice function for arrays backed by Blosc has been implemented.
This allows users to  update the values in the array whenever and wherever
they want.

Finally, some constructors (empty, zeros, full) have been introduced using the
special-values features introduced in Blosc.

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
