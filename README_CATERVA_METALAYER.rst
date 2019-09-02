Caterva MetaLayer
===================

Caterva is storing persistently its information about shapes and partition shapes as an additional layer in the Blosc2 frame format.

Caterva metalayer follows the msgpack format::

    |-0-|-1-|-2-|~~~~~~~~~~~~~~~~|---|~~~~~~~~~~~~~~~~|
    | 9X| nd| 9X| shape          | 9X| partshape      |
    |---|---|---|~~~~~~~~~~~~~~~~|---|~~~~~~~~~~~~~~~~|
      ^   ^   ^                     ^
      |   |   |                     |
      |   |   |                     +--[msgpack] positive fixnum for the number of dimensions (nd, up to 127)
      |   |   +---[msgpack] fixarray with X=nd elements
      |   +------[msgpack] positive fixnum for the number of dimensions (nd, up to 127)
      +---[msgpack] fixarray with X=3 elements

The shape section
-----------------

This section is meant to store the actual shape info.  There are as many fields as `nd` dimensions::

    |---|--8 bytes---|---|--8 bytes---|~~~~~|---|--8 bytes---|
    | d3| first_dim  | d3| second_dim | ... | d3| last_dim   |
    |---|------------|---|------------|~~~~~|---|------------|
      ^                ^                      ^
      |                |                      |
      |                |                      +--[msgpack] int64
      |                +------[msgpack] int64
      +---[msgpack] int64

The partshape section
---------------------

This section is meant to store the actual partition shape info.  There are as many fields as `nd` dimensions::

    |---|--4 bytes---|---|--4 bytes---|~~~~~|---|--4 bytes---|
    | d2| first_dim  | d2| second_dim | ... | d2| last_dim   |
    |---|------------|---|------------|~~~~~|---|------------|
      ^                ^                      ^
      |                |                      |
      |                |                      +--[msgpack] int32
      |                +------[msgpack] int32
      +---[msgpack] int32
