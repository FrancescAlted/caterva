API description
===============
This section contains the Caterva public API and the structures needed to use it. Caterva tries
to be backward compatible with both the C-Blosc1 API and format.


Global constants
----------------
..  doxygendefine:: CATERVA_MAXDIM
..  doxygendefine:: CATERVA_METALAYER_VERSION


Configuration parameters
------------------------
..  doxygentypedef:: caterva_config_t
..  doxygenstruct:: caterva_config_s
    :members:


Context
-------
..  doxygentypedef:: caterva_context_t

Creation and destruction
++++++++++++++++++++++++
..  doxygenfunction:: caterva_context_new

..  doxygenfunction:: caterva_context_free


Container
---------
..  doxygentypedef:: caterva_array_t

General parameters
++++++++++++++++++
..  doxygentypedef:: caterva_params_t
..  doxygenstruct:: caterva_params_s
    :members:


Storage parameters
++++++++++++++++++
..  doxygentypedef:: caterva_storage_t
..  doxygenstruct:: caterva_storage_s
    :members:

.. doxygenunion:: caterva_storage_properties_u