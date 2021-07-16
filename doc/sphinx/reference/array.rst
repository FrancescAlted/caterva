Array
=====
A Caterva array is a digital, ordered and multidimensional dataset (compressed or not) easy to handle using the Caterva functions.
Furthermore, these functions let us to perform oprations with Caterva arrays like copying them, slicing them or converting them into buffers or files and vice versa.

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

.. doxygenstruct:: caterva_metalayer_t
   :members:

.. doxygenstruct:: caterva_storage_properties_plainbuffer_t
   :members:


Creation
--------

Fast constructors
+++++++++++++++++

.. doxygenfunction:: caterva_empty

.. doxygenfunction:: caterva_zeros

.. doxygenfunction:: caterva_full


From/To buffer
++++++++++++++
.. doxygenfunction:: caterva_from_buffer

.. doxygenfunction:: caterva_to_buffer


From/To file
++++++++++++

.. doxygenfunction:: caterva_open
.. doxygenfunction:: caterva_save


From Blosc object
+++++++++++++++++

.. doxygenfunction:: caterva_from_schunk

.. doxygenfunction:: caterva_from_serial_schunk


Copying
+++++++

.. doxygenfunction:: caterva_copy


Slicing
-------

.. doxygenfunction:: caterva_get_slice_buffer

.. doxygenfunction:: caterva_set_slice_buffer

.. doxygenfunction:: caterva_get_slice

.. doxygenfunction:: caterva_squeeze


Destruction
-----------

.. doxygenfunction:: caterva_free

.. doxygenfunction:: caterva_remove
