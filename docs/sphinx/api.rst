API description
===============

This section contains the Caterva public API and the structures needed to use it. Caterva tries
to be backward compatible with both the C-Blosc1 API and format.


Utility variables
+++++++++++++++++

..  doxygendefine:: CATERVA_MAXDIM
..  doxygendefine:: CATERVA_METALAYER_VERSION

Utility structs
+++++++++++++++

..  doxygenstruct:: caterva_dims_t
    :members:

Context functions
+++++++++++++++++

..  doxygenstruct:: caterva_ctx_t
    :members:

..  doxygenfunction:: caterva_new_ctx

..  doxygenfunction:: caterva_free_ctx


Creation functions
++++++++++++++++++

..  doxygenstruct:: caterva_array_t
    :members:

..  doxygenfunction:: caterva_empty_array

..  doxygenfunction:: caterva_append

..  doxygenfunction:: caterva_from_frame

..  doxygenfunction:: caterva_from_sframe

..  doxygenfunction:: caterva_from_file

..  doxygenfunction:: caterva_from_buffer

..  doxygenfunction:: caterva_copy

..  doxygenfunction:: caterva_free_array


Slicing
+++++++

.. doxygenfunction:: caterva_get_slice

.. doxygenfunction:: caterva_get_slice_buffer

.. doxygenfunction:: caterva_set_slice_buffer

.. doxygenfunction:: caterva_squeeze
