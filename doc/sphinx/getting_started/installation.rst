Installation
============

Caterva can be built, tested and installed using CMake. The following procedure
describes a typical CMake build.

1. Download the source code from Github:

    .. tab:: Unix

        .. code-block::

            git clone --recurse-submodules git@github.com:Blosc/caterva.git

    .. tab:: Windows

        .. code-block::

            git clone --recurse-submodules git@github.com:Blosc/caterva.git



2. Create the build directory inside the sources and move into it:

    .. tab:: Unix

        .. code-block::

            cd caterva
            mkdir build
            cd build

    .. tab:: Windows

        .. code-block::

            cd caterva
            mkdir build
            cd build


3. Now run CMake configuration:

    .. tab:: Unix

        .. code-block::

            cmake -DCMAKE_BUILD_TYPE='Debug/Release' ..

    .. tab:: Windows

        .. code-block::

            cmake ..


4. Build and test Caterva:

    .. tab:: Unix

        .. code-block::

            cmake --build .
            ctest

    .. tab:: Windows

        .. code-block::

            cmake --build . --config 'Debug/Release'
            ctest --build-config 'Debug/Release'


5. If desired, install Caterva:

    .. tab:: Unix

        .. code-block::

            cmake --build . --target install


    .. tab:: Windows

        .. code-block::

            cmake --build . --target install --config 'Debug/Release'


That's all folks!
