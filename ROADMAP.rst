Roadmap
=======

Caterva is a multidimensional container for binary data.  It is a pure C library, allowing for better interoperatibility between different languages (although Python stands out high in the list).

This document lists the goals for a production release of Caterva.


Existing features
-----------------

* **Built on top of Blosc2:** besides transparent compression, this allows to store large compressed datasets either in-memory or on-disk. In addition, Caterva inherits all the improvements that are being introduced in Blosc2 (See https://github.com/Blosc/c-blosc2/blob/master/ROADMAP.md).

* **Two-level multidimensional partitions:** like other libraries, Caterva stores the data into multidimensional chunks for efficient slicing. But in addition, Caterva introduces a new level of partitioning.  Within each chunk, the data is re-chunked into smaller multidimensional sets called blocks, leading to more fine-grained, and hence, to [even more efficient slicing capabilities](https://github.com/Blosc/cat4py/blob/master/notebooks/slicing-performance.ipynb).

* **Plainbuffer support:** Caterva also allows to store data in a contiguous buffer. In this way, it facilitates the interoperability with other libraries like NumPy.


Actions to be done
------------------

* **Update values:** this will make array creation more flexible, since the process of populating an array will not necessarily need to follow a row-wise order (it can actually be any order).

* **Resize array dimensions:** this will allow to increase or decrease in size any dimension of the arrays.

* **Improve slicing capabilties:** currently Caterva only supports basic slicing based on `start:stop` ranges; we would like to extend this to `start:stop:step` as well as selections based on an array of booleans (similar to NumPy).

* **Add support for [DLPack](https://github.com/dmlc/dlpack):** support for DLPack would be nice for being able to share data between different frameworks and devices.  This should complement (or even replace in the long term) the existing plainbuffer support.

* **Support for multidimensional filters:** this will improve the in-memory spatial locally for data that is **n-dim closer** in the array; by n-dim closer we mean that the multidimensional norm (in an Euclidean space) between two different positions of elements is shorter.  This may led to better compression opportunities when spatial locality (Euclidean space) is high.

* **Provide wheels:** this will make the installation much more easier for the user.


Outreaching
-----------

* **Improve the main Caterva README:** this should allow a better overview at first glance of all the features that Caterva offers right now.

* **Attend to meetings and conferences:** it is very important to plan going to conferences for advertising Caterva and meeting people in-person.  We need to decide which meetings to attend.  As there are not that much conferences about C libraries, it is important to leverage the [cat4py](https://github.com/Blosc/cat4py) wrapper so as to try to promote Caterva on Python conferences too.
  
* Other outreaching activities would be to produce videos of the kind 'Caterva in 10 minutes', or producing compelling tutorials (preferably based on Jupyter notebook, and using services like [binder](https://mybinder.org) that allow a low entry level for quick trials).
