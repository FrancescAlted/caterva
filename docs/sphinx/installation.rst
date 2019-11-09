Installation
============


Building Caterva with CMake
+++++++++++++++++++++++++++

Caterva can be built, tested and installed using CMake. The following procedure describes a
typical CMake build. In order to install Caterva, you need to have the library c-blosc2 builded
or installed.

1. Create the build directory inside the sources and move into it::

    ```
    cd c-blosc2-sources
    mkdir build/
    cd build/
    ```

2. Now run CMake configuration and, if necessary, specify the directory where the Blosc build is
and the directory where the Blosc headers are::

    ```
    cmake -DBLOSC_DIR='blosc_build_dir' -DBLOSC_INCLUDE='blosc_headers_dir' ..
    ```

3. Build and test Caterva::

    ```
    cmake --build .
    ctest
    ```

4. If desired, install Caterva::

    ```
    cmake --build . --target install
    ```


That's all folks!