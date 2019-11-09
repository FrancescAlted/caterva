API description
===============

This section contains the Caterva public API and the structures needed to use it. Caterva tries
to be backward compatible with both the C-Blosc1 API and format.


Utility variables
+++++++++++++++++

.. doxygendefine:: CATERVA_MAXDIM
.. doxygendefine:: CATERVA_METALAYER_VERSION


Context functions
+++++++++++++++++

.. doxygenstruct:: caterva_ctx_t
   :members:

.. doxygenfunction:: caterva_new_ctx

.. doxygenfunction:: caterva_free_ctx

Wrappers
++++++++

- `cat4py <https://github.com/Blosc/cat4py>`_: a Pythonic wrapper of Caterva.