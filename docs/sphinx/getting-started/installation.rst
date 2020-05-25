Installation
============

Building Caterva with CMake
+++++++++++++++++++++++++++

Caterva can be built, tested and installed using CMake. The following procedure describes a
typical CMake build. In order to install Caterva, you need to have the library c-blosc2 builded
or installed.


Unix
::::

1. Create the build directory inside the sources and move into it::

    cd caterva-sources
    mkdir build/
    cd build/

2. Now run CMake configuration and, if necessary, specify the directory where the Blosc build is and the directory where the Blosc headers are::

        cmake -DCMAKE_BUILD_TYPE='Debug/Release' -DBLOSC_DIR='blosc_build_dir' -DBLOSC_INCLUDE='blosc_headers_dir' ..

3. Build and test Caterva::

    cmake --build .
    ctest

4. If desired, install Caterva::

    cmake --build . --target install



Windows
:::::::

1. Create the build directory inside the sources and move into it::

    cd caterva-sources
    mkdir build/
    cd build/

2. Now run CMake configuration and, if necessary, specify the directory where the Blosc build is and the directory where the Blosc headers are::

        cmake -DBLOSC_DIR='blosc_build_dir' -DBLOSC_INCLUDE='blosc_headers_dir' ..

3. Build and test Caterva::

    cmake --build . --config 'Debug/Release'
    ctest -C 'Debug/Release'

4. If desired, install Caterva::

    cmake --build . --target install --config 'Debug/Release'



That's all folks!
