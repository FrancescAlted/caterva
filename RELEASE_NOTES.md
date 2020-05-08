# Release notes for Caterva

## Changes from 0.3.0 to 0.3.3

* Fixing that 0.3.1 and 0.3.2 tags were not made in master :-/

## Changes from 0.2.2 to 0.3.0

* Big code and API refactorization.  As result, the API is more consistent and hopefully more intuitive to use.  For more info on the new API, see https://caterva.readthedocs.io.

## Changes from 0.2.1 to 0.2.2

* Add a `caterva_from_sframe()` function. 

## Changes from 0.2.0 to 0.2.1

* Both static and dynamic libraries are created by default now. If you want to disable the creation
of one of them, just set the cmake options `SHARED_LIB=OFF` or `STATIC_LIB=OFF`.

* Add a `copy` parameter to `caterva_from_file()`.
