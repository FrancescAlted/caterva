Metalayers
==========
Metalayers are metadata for informing about the properties of data that is stored on a container. Caterva implements its own metalayer on top of C-Blosc2 for storing multidimensional information.
In general, you can use metalayers for adapting Blosc2 containers (and in particular, Caterva arrays) to your own needs.

Fixed-length metalayers
-----------------------
These small metalayers are stored in the header of the frame. This allows adding chunks to the frame without the need to rewrite the whole meta information and data coming after it.
However, fixed-length metalayers cannot be resized and once the first chunk of data is added to the super-chunk, no more fixed-length metalayers can be added either.

.. doxygenfunction:: caterva_meta_exists

.. doxygenfunction:: caterva_meta_get

.. doxygenfunction:: caterva_meta_update


Variable-length metalayers
--------------------------
Unlike fixed-length metalayers, these can be resized and are stored in the trailer section of the frame.
This provides users a lot of flexibility to define their own metadata.

.. doxygenfunction:: caterva_vlmeta_add

.. doxygenfunction:: caterva_vlmeta_exists

.. doxygenfunction:: caterva_vlmeta_get

.. doxygenfunction:: caterva_vlmeta_update
