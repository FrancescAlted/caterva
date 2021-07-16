Caterva has an internal structure managed by parameters that record the different properties of each dataset.
The most fundamental parts of this structure are the Caterva context and the configuration parameters it depends on.

Context
=======
..  doxygenstruct:: caterva_ctx_t


Configuration parameters
++++++++++++++++++++++++

..  doxygenstruct:: caterva_config_t
    :members:

.. doxygenvariable:: CATERVA_CONFIG_DEFAULTS


Creation
++++++++
..  doxygenfunction:: caterva_ctx_new


Destruction
+++++++++++

..  doxygenfunction:: caterva_ctx_free
