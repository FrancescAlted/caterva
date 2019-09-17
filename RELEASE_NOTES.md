# Release notes for Caterva 0.2.1

## Changes from 0.2.0 to 0.2.1

* Both static and dynamic libraries are created by default now. If you want to disable the creation
of one of them, just set the cmake options `SHARED_LIB=OFF` or `STATIC_LIB=OFF`.

* Add a `copy` parameter to `caterva_from_file()`.
