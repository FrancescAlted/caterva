Release notes
=============

Changes from 0.4.0 to 0.4.1
---------------------------

* Redesign the caterva guts and perform a code refactorization in order
  to simplify the code. This includes API renaming. A performance improvement
  is obtained as can be seen in https://github.com/Blosc/caterva/pull/69#issuecomment-843835622

* Implement a set_slice for arrays backed by Blosc. This allows users to
  update the values in the array whenever and wherever they want.

* Implement constructors (empty, zeros, full) using the special-values
  features introduced in Blosc.

* Use the `pydata_sphinx_theme` in the documentation.


Changes from 0.3.3 to 0.4.0
---------------------------

* API renaming. The function names simulate a *namespace* and some
  variable names have been changed to those used by the general community
  (*part -> chunk*).

* Add a new level of multi-dimensionality. As a result, unlike other
  libraries, Caterva supports two levels of multi-dimensional chunking (chunks
  and blocks).

* Improve library compilation to allow users to avoid building tests and
  examples.

* Simplify the test's suite for a proper integration in Windows.

* Update documentation to improve the library description and to add the
  *Release notes* and the *Roadmap* in a new section.

Changes from 0.3.0 to 0.3.3
---------------------------

* Fixing that 0.3.1 and 0.3.2 tags were not made in master :-/

Changes from 0.2.2 to 0.3.0
---------------------------

* Big code and API refactorization.  As result, the API is more consistent and
  hopefully more intuitive to use.  For more info on the new API, see
  https://caterva.readthedocs.io.

Changes from 0.2.1 to 0.2.2
---------------------------

* Add a `caterva_from_sframe()` function. 

Changes from 0.2.0 to 0.2.1
---------------------------

* Both static and dynamic libraries are created by default now.  If you want to
  disable the creation of one of them, just set the cmake options
  `SHARED_LIB=OFF` or `STATIC_LIB=OFF`.

* Add a `copy` parameter to `caterva_from_file()`.
