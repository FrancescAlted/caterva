API description
===============

This section contains the Caterva public API and the structures needed to use it. Caterva tries
to be backward compatible with both the C-Blosc1 API and format.


Global constants
++++++++++++++++

..  doxygendefine:: CATERVA_MAXDIM
..  doxygendefine:: CATERVA_METALAYER_VERSION


Shapes, partitions and coordinates
++++++++++++++++++++++++++++++++++

..  doxygenstruct:: caterva_dims_s
    :members:

..  doxygenstruct:: caterva_shape_t

..  doxygenstruct:: caterva_pshape_t
    :members:

.. doxygenfunction:: caterva_new_dims

.. doxygenfunction:: caterva_get_shape

.. doxygenfunction:: caterva_get_pshape


Context
+++++++

..  doxygenstruct:: caterva_ctx_t
    :members:

..  doxygenfunction:: caterva_new_ctx

..  doxygenfunction:: caterva_free_ctx


Creation
++++++++

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

