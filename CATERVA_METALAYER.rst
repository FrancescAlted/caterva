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
