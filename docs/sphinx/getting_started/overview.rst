What is caterva?
================

Caterva is a container for multidimensional data that is specially designed to
read, in an incredibly efficient way, slices of it. To achieve this, a new
chunking-based data layout has been created.

.. image:: overview.png
   :width: 90%
   :align: center

Like other libraries like zarr, HDF5 or TileDB, Caterva stores the data into
multidimensional chunks (yellow cubes). These chunks can then be read
individually, improving performance when reading slices of the dataset. But
also, Caterva introduces a new level of chunking. Within each chunk, the data is
re-partitioned into smaller multidimensional sets called blocks (green cubes).
In this way, Caterva can read blocks individually (and also in parallel) instead
of chunks.

These partition levels allow to access data efficiently with a larger set of
data access patterns. This is due to obtain the desired slice, instead of
reading the data using the chunks, data are obtained using the blocks.

Blosc
-----

In Caterva the compression is handled transparently for the user by leveraging
the Blosc2 library. Blosc is an extremely fast compressor specially designed
for binary data. It uses the blocking technique to reduce activity on the memory
bus as much as possible. It also leverages SIMD (SSE2, AVX2 for Intel, NEON for
ARM, Altivec for Power) and multi-threading capabilities present in multi-core
processors so as to accelerate the compression/decompression process to a
maximum.

Being able to store in an in-memory data container does not mean that data
cannot be persisted. It is critical to find a way to store and retrieve data
efficiently. Also, it is important to adopt open formats for reducing the
maintenance burden and facilitate its adoption more quickly. Blosc2 brings such
an efficient and open format for `persistency <https://github
.com/Blosc/c-blosc2/blob/master/README_FRAME_FORMAT.rst>`__.

An aditional feature that introduces Blosc2 is the concept of metalayers. They
are small metadata for informing about the kind of data that is stored on a
Blosc2 container. They are handy for defining layers with different specs: data
types, geo-spatial...

.. include:: ../../../CATERVA_METALAYER.rst
