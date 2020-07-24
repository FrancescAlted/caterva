Roadmap
=======

This document lists the main goals for the upcoming Caterva releases.

Actions to be done
------------------

* Do a complete refactorization of the Caterva C code to facilitate its usability. For example, after that refactorization, the shape will be passed to the container when it is created.

* Adapt the Python interface to the refactorization done in C code.

* Add examples into the Python wrapper documentation and create some jupyter notebooks.

* Build wheels to make the installation easier for the user.

* Add a new level of multidimensionality in Caterva. After that, we will support three layers of multidimensionality in a Caterva container: the shape, the chunk shape and the block shape.
