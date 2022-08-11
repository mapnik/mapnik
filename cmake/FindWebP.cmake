# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindWebP
-------

Finds the WebP library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``WebP::WebP``
  The WebP library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``WebP_FOUND``
  True if the system has the WebP library.
``WebP_VERSION``
  The version of the WebP library which was found.
``WebP_INCLUDE_DIRS``
  Include directories needed to use WebP.
``WebP_LIBRARIES``
  Libraries needed to link to WebP.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``WebP_INCLUDE_DIR``
  The directory containing ``decode.h``.
``WebP_LIBRARY``
  The path to the Foo library.

#]=======================================================================]

if(NOT WebP_LIBRARY)
    find_package(PkgConfig QUIET)
    pkg_check_modules(PC_WebP QUIET libwebp)
    set(WebP_VERSION ${PC_WebP_VERSION})
    find_path(WebP_INCLUDE_DIR NAMES decode.h HINTS ${PC_WebP_INCLUDEDIR} ${PC_WebP_INCLUDE_DIR} PATH_SUFFIXES webp)
    find_library(WebP_LIBRARY_RELEASE NAMES ${WebP_NAMES} webp HINTS ${PC_WebP_LIBDIR} ${PC_WebP_LIBRARY_DIRS})
    find_library(WebP_LIBRARY_DEBUG NAMES ${WebP_NAMES} webpd HINTS ${PC_WebP_LIBDIR} ${PC_WebP_LIBRARY_DIRS})
    include(SelectLibraryConfigurations)
    select_library_configurations(WebP)
else()
  file(TO_CMAKE_PATH "${WebP_LIBRARY}" WebP_LIBRARY)
endif()

if ("${WebP_FIND_VERSION}" VERSION_GREATER "${WebP_VERSION}")
    if (WebP_VERSION)
        message(FATAL_ERROR "Required version (" ${WebP_FIND_VERSION} ") is higher than found version (" ${PC_WebP_VERSION} ")")
    else ()
        message(WARNING "Cannot determine WebP version without pkg-config")
    endif ()
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WebP
  REQUIRED_VARS
    WebP_LIBRARY
    WebP_INCLUDE_DIR
  VERSION_VAR WebP_VERSION
)
mark_as_advanced(WebP_INCLUDE_DIR WebP_LIBRARY)

if (WebP_FOUND)
  set(WebP_LIBRARIES ${WebP_LIBRARY})
  set(WebP_INCLUDE_DIRS ${WebP_INCLUDE_DIR})
  if(NOT TARGET WebP::WebP)
    add_library(WebP::WebP UNKNOWN IMPORTED)
    set_target_properties(WebP::WebP PROPERTIES
                          INTERFACE_INCLUDE_DIRECTORIES ${WebP_INCLUDE_DIR}
                          IMPORTED_LINK_INTERFACE_LANGUAGES C)

    if(WebP_LIBRARY_RELEASE)
      set_property(TARGET WebP::WebP APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
      set_target_properties(WebP::WebP PROPERTIES IMPORTED_LOCATION_RELEASE "${WebP_LIBRARY_RELEASE}")
    endif()

    if(WebP_LIBRARY_DEBUG)
      set_property(TARGET WebP::WebP APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
      set_target_properties(WebP::WebP PROPERTIES IMPORTED_LOCATION_DEBUG "${WebP_LIBRARY_DEBUG}")
    endif()

    if(NOT WebP_LIBRARY_RELEASE AND NOT WebP_LIBRARY_DEBUG)
        set_target_properties(WebP::WebP PROPERTIES IMPORTED_LOCATION "${WebP_LIBRARY}")
    endif()
  endif()
endif ()
