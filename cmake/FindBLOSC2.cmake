# - Try to find c-blosc2
# Once done this will define
#  BLOSC2_FOUND - System has c-blosc2
#  BLOSC2_INCLUDE_DIRS - The c-blosc2 include directories
#  BLOSC2_LIBRARIES - The shared libraries required for using c-blosc2
#  BLOSC2_DEFINITIONS -
#
# Advanced variables
#  BLOSC2_INCLUDE_DIR - Used to set BLOSC2_INCLUDE_DIRS
#  BLOSC2_LIBRARY - Used to set BLOSC2_LIBRARIES
#  BLOSC2_LIBRARY_STATIC - Name of the static library ending in .a or .lib
#  
# Based upon the LibXml2 example in the CMake wiki:
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/tutorials/How-To-Find-Libraries#writing-find-modules

find_package(PkgConfig)
pkg_check_modules(PC_BLOSC2 QUIET blosc)
set(BLOSC2_DEFINITIONS ${PC_BLOSC2_CFLAGS_OTHER})

find_path(BLOSC2_INCLUDE_DIR blosc2.h
          HINTS ${PC_BLOSC2_INCLUDEDIR} ${PC_BLOSC2_INCLUDE_DIRS} )
 
find_library(BLOSC2_LIBRARY NAMES blosc2
             HINTS ${PC_BLOSC2_LIBDIR} ${PC_BLOSC2_LIBRARY_DIRS} )
find_library(BLOSC2_LIBRARY_STATIC NAMES libblosc2.a libblosc2.lib
             HINTS ${PC_BLOSC2_LIBDIR} ${PC_BLOSC2_LIBRARY_DIRS} )

if(BLOSC2_INCLUDE_DIR AND BLOSC2_LIBRARY)
    # Debugging messages
    message(VERBOSE "Found BLOSC2 include dir: ${BLOSC2_INCLUDE_DIR}")
    message(VERBOSE "Found BLOSC2 library: ${BLOSC2_LIBRARY}")
endif()

if(BLOSC2_LIBRARY_STATIC)
    set(BLOSC2_STATIC_FOUND TRUE)
    message(VERBOSE "Found BLOSC2 static library: ${BLOSC2_LIBRARY_STATIC}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BLOSC2 DEFAULT_MSG BLOSC2_INCLUDE_DIR BLOSC2_LIBRARY)

mark_as_advanced(BLOSC2_INCLUDE_DIR BLOSC2_LIBRARY BLOSC2_LIBRARY_STATIC)

set(BLOSC2_LIBRARIES ${BLOSC2_LIBRARY})
set(BLOSC2_INCLUDE_DIRS ${BLOSC2_INCLUDE_DIR})
