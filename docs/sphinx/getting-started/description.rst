Package overview
================

Caterva is a container for multidimensional data that is specially designed to read, in an
incredibly efficient way, slices of data. To achieve this, a new chunking-based data layout
has been created.

.. image:: overview.png
   :width: 40%
   :align: center

Like other libraries like zarr or hdf5, Caterva stores the data into multidimensional chunks
(yellow cubes). These chunks can then be read individually, improving performance when
reading slices of the dataset. But also, Caterva introduces a new level
of chunking. Within each chunk, the data is re-partitioned into smaller multidimensional
sets called blocks (green cubes). In this way, Caterva can read blocks individually
(and also in parallel) instead of chunks.

These partition levels, that emulate the memory hierarchy of modern computers, allow access
to data in a more efficient way. This is due to obtain the desired slice, instead of reading
the data using the chunks, data are obtained using the blocks.

Blosc
-----

In Caterva the compression is handled transparently for the user by leveraging the Blosc2 library.
Blosc is an extremely fast compressor specially designed for binary data. It uses the blocking
technique to reduce activity on the memory bus as much as possible. It also leverages SIMD
(SSE2, AVX2 for Intel, NEON for ARM, Altivec for Power) and multi-threading capabilities
present in multi-core processors so as to accelerate the compression/decompression process
to a maximum.

Being able to store in an in-memory data container does not mean that data cannot be persisted.
It is critical to find a way to store and retrieve data efficiently. Also, it is important to
adopt open formats for reducing the maintenance burden and facilitate its adoption more quickly.
Blosc2 brings such an efficient and open format for
`persistency <https://github.com/Blosc/c-blosc2/blob/master/README_FRAME_FORMAT.rst>`__.

An aditional feature that introduces Blosc2 is the concept of metalayers. They are small metadata
for informing about the kind of data that is stored on a Blosc2 container. They are handy for
defining layers with different specs: data types, geo-spatial...

Caterva metalayer
+++++++++++++++++

Caterva is created by specifying a metalayer on top of a Blosc2 container for storing
multidimensional information. This metalayer can be modified so that the shapes can be updated
(e.g. an array can grow or shrink). Specifically, Caterva metalayer follows the msgpack format::

    |-0-|-1-|-2-|-3-|~~~~~~~~~~~~~~~~|---|~~~~~~~~~~~~~~~~|---|~~~~~~~~~~~~~~~~|
    | 9X| n | n | 9X| shape          | 9X| chunkshape     | 9X| blockshape     |
    |---|---|---|---|~~~~~~~~~~~~~~~~|---|~~~~~~~~~~~~~~~~|---|~~~~~~~~~~~~~~~~|
      ^   ^   ^   ^                    ^                    ^
      |   |   |   |                    |                    |
      |   |   |   |                    |                    +--[msgpack] positive fixnum for n
      |   |   |   |                    +--[msgpack] positive fixnum for n
      |   |   |   +--[msgpack] fixarray with X=nd elements
      |   |   +--[msgpack] positive fixnum for the number of dimensions (n, up to 127)
      |   +--[msgpack] positive fixnum for the metalayer format version (up to 127)
      +---[msgpack] fixarray with X=5 elements

In this format, the shape section is meant to store the actual shape info::

    |---|--8 bytes---|---|--8 bytes---|~~~~~|---|--8 bytes---|
    | d3| first_dim  | d3| second_dim | ... | d3| nth_dim    |
    |---|------------|---|------------|~~~~~|---|------------|
      ^                ^                      ^
      |                |                      |
      |                |                      +--[msgpack] int64
      |                +--[msgpack] int64
      +--[msgpack] int64


Next, the chunkshape section is meant to store the actual chunk shape info::

    |---|--4 bytes---|---|--4 bytes---|~~~~~|---|--4 bytes---|
    | d2| first_dim  | d2| second_dim | ... | d2| nth_dim    |
    |---|------------|---|------------|~~~~~|---|------------|
      ^                ^                      ^
      |                |                      |
      |                |                      +--[msgpack] int32
      |                +--[msgpack] int32
      +--[msgpack] int32

Finally, the blockshape section is meant to store the actual block shape info::

    |---|--4 bytes---|---|--4 bytes---|~~~~~|---|--4 bytes---|
    | d2| first_dim  | d2| second_dim | ... | d2| nth_dim    |
    |---|------------|---|------------|~~~~~|---|------------|
      ^                ^                      ^
      |                |                      |
      |                |                      +--[msgpack] int32
      |                +--[msgpack] int32
      +--[msgpack] int32


