ROADMAP for Caterva
===================

Caterva is a C library for handling multi-dimensional, compressed datasets in an easy and convenient manner.  This document lists the goals for a production release of Caterva.


Actions to be done
------------------

* Do a complete refactorization of the Caterva C code to facilitate its usability. For example, after that refactorization, the shape will be passed to the container when it is created.

* Adapt the Python interface to the refactorization done in C code.

* Add examples into the Python wrapper documentation and create some jupyter notebooks.

* Build wheels to make the installation easier for the user.

* Add a new level of multidimensionality in Caterva. After that, we will support three layers of multidimensionality in a Caterva container: the shape, the chunk shape and the block shape.
