# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindCairo
-----------

Find Cairo 2D graphics library.


Imported Targets
^^^^^^^^^^^^^^^^
This module defines :prop_tgt:`IMPORTED` target ``Cairo::Cairo``, if
cairo has been found.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``CAIRO_FOUND``
  True if cairo headers and library were found.
``CAIRO_INCLUDE_DIRS``
  Directory where cairo headers are located.
``CAIRO_LIBRARIES``
  cairo libraries to link against.
``CAIRO_VERSION_MAJOR``
  The major version of cairo
``CAIRO_VERSION_MINOR``
  The minor version of cairo
``CAIRO_VERSION_PATCH``
  The patch version of cairo
``CAIRO_VERSION_STRING``
  version number as a string (ex: "1.16.0")
#]=======================================================================]

if(NOT CAIRO_LIBRARY)
  find_path(CAIRO_INCLUDE_DIR NAMES cairo.h HINTS ${PC_CAIRO_INCLUDEDIR} ${PC_CAIRO_INCLUDE_DIR} PATH_SUFFIXES cairo)
  find_library(CAIRO_LIBRARY_RELEASE NAMES ${Cairo_NAMES} cairo HINTS ${PC_CAIRO_LIBDIR} ${PC_CAIRO_LIBRARY_DIRS})
  find_library(CAIRO_LIBRARY_DEBUG NAMES ${Cairo_NAMES} cairod HINTS ${PC_CAIRO_LIBDIR} ${PC_CAIRO_LIBRARY_DIRS})
  include(SelectLibraryConfigurations)
  select_library_configurations(CAIRO)
else()
  file(TO_CMAKE_PATH "${CAIRO_LIBRARY}" CAIRO_LIBRARY)
endif()

if(CAIRO_INCLUDE_DIR AND NOT CAIRO_VERSION)
  if(EXISTS "${CAIRO_INCLUDE_DIR}/cairo-version.h")
      file(READ "${CAIRO_INCLUDE_DIR}/cairo-version.h" CAIRO_VERSION_CONTENT)

      string(REGEX MATCH "#define +CAIRO_VERSION_MAJOR +([0-9]+)" _dummy "${CAIRO_VERSION_CONTENT}")
      set(CAIRO_VERSION_MAJOR "${CMAKE_MATCH_1}")

      string(REGEX MATCH "#define +CAIRO_VERSION_MINOR +([0-9]+)" _dummy "${CAIRO_VERSION_CONTENT}")
      set(CAIRO_VERSION_MINOR "${CMAKE_MATCH_1}")

      string(REGEX MATCH "#define +CAIRO_VERSION_MICRO +([0-9]+)" _dummy "${CAIRO_VERSION_CONTENT}")
      set(CAIRO_VERSION_PATCH "${CMAKE_MATCH_1}")

      set(CAIRO_VERSION "${CAIRO_VERSION_MAJOR}.${CAIRO_VERSION_MINOR}.${CAIRO_VERSION_PATCH}")
      set(CAIRO_VERSION_STRING ${CAIRO_VERSION})
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Cairo
  REQUIRED_VARS
    CAIRO_LIBRARY
    CAIRO_INCLUDE_DIR
    VERSION_VAR
    CAIRO_VERSION_STRING
)
mark_as_advanced(CAIRO_INCLUDE_DIR CAIRO_LIBRARY)

if (CAIRO_FOUND)
  set(CAIRO_LIBRARIES ${CAIRO_LIBRARY})
  set(CAIRO_INCLUDE_DIRS ${CAIRO_INCLUDE_DIR})
  if(NOT TARGET Cairo::Cairo)
    add_library(Cairo::Cairo UNKNOWN IMPORTED)
    set_target_properties(Cairo::Cairo PROPERTIES
                          INTERFACE_INCLUDE_DIRECTORIES ${CAIRO_INCLUDE_DIR}
                          IMPORTED_LINK_INTERFACE_LANGUAGES C)

    if(CAIRO_LIBRARY_RELEASE)
      set_property(TARGET Cairo::Cairo APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
      set_target_properties(Cairo::Cairo PROPERTIES IMPORTED_LOCATION_RELEASE "${CAIRO_LIBRARY_RELEASE}")
    endif()

    if(CAIRO_LIBRARY_DEBUG)
      set_property(TARGET Cairo::Cairo APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
      set_target_properties(Cairo::Cairo PROPERTIES IMPORTED_LOCATION_DEBUG "${CAIRO_LIBRARY_DEBUG}")
    endif()

    if(NOT CAIRO_LIBRARY_RELEASE AND NOT CAIRO_LIBRARY_DEBUG)
        set_target_properties(Cairo::Cairo PROPERTIES IMPORTED_LOCATION "${CAIRO_LIBRARY}")
    endif()
  endif()
endif ()
