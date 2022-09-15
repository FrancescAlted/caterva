Array
=====
A Caterva array is a multidimensional dataset (compressed or not) easy to handle using the Caterva functions.
Furthermore, Caterva only stores item size instead of the data type and every item of a Caterva array has the same size.
On the other hand, Caterva functions let users to perform different operations with these arrays like copying, slicing, setting them or converting them into buffers or files and vice versa.

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

.. doxygenstruct:: caterva_metalayer_t
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


From/To cframe buffer
+++++++++++++++++++++

.. doxygenfunction:: caterva_from_cframe

.. doxygenfunction:: caterva_to_cframe

Copying
+++++++

.. doxygenfunction:: caterva_copy


Slicing
-------

.. doxygenfunction:: caterva_get_slice_buffer

.. doxygenfunction:: caterva_set_slice_buffer

.. doxygenfunction:: caterva_get_slice

.. doxygenfunction:: caterva_squeeze

.. doxygenfunction:: caterva_squeeze_index


Destruction
-----------

.. doxygenfunction:: caterva_free

.. doxygenfunction:: caterva_remove
