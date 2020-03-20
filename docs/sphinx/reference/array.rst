Caterva Array
=============

.. doxygenstruct:: caterva_array_t


Parameters
----------


General parameters
++++++++++++++++++

.. doxygenstruct:: caterva_params_t
   :members:


Storage parameters
++++++++++++++++++
.. doxygenstruct:: caterva_storage_t
   :members:

.. doxygenenum:: caterva_storage_backend_t

.. doxygenunion:: caterva_storage_properties_t

.. doxygenstruct:: caterva_storage_properties_blosc_t
   :members:

.. doxygenstruct:: caterva_storage_properties_plainbuffer_t
   :members:


Creation
--------


Chunk by chunk
++++++++++++++
.. doxygenfunction:: caterva_array_empty

.. doxygenfunction:: caterva_array_append


From/To buffer
++++++++++++++
.. doxygenfunction:: caterva_array_from_buffer

.. doxygenfunction:: caterva_array_to_buffer


From/To frame
+++++++++++++
.. doxygenfunction:: caterva_array_from_frame

.. doxygenfunction:: caterva_array_from_sframe

From/To file
++++++++++++
.. doxygenfunction:: caterva_array_from_file

Copying
-------

.. doxygenfunction:: caterva_array_copy


Slicing
-------

.. doxygenfunction:: caterva_array_get_slice

.. doxygenfunction:: caterva_array_get_slice_buffer

.. doxygenfunction:: caterva_array_set_slice_buffer

.. doxygenfunction:: caterva_array_squeeze


Destruction
-----------

.. doxygenfunction:: caterva_array_free