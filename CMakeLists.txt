cmake_minimum_required(VERSION 3.15.0)
project(mapnik 
    VERSION 3.0.24 
    HOMEPAGE_URL "https://mapnik.org/" 
    DESCRIPTION "Mapnik is an open source toolkit for developing mapping applications" 
    LANGUAGES CXX 
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(BUILD_SHARED_LIBS ON)
option(USE_EXTERNAL_MAPBOX_GEOMETRY "Use a external mapnik/geometry.hpp. If off, use the submodule" OFF)
option(USE_EXTERNAL_MAPBOX_POLYLABEL "Use a external mapnik/polylabel. If off, use the submodule" OFF)
option(USE_EXTERNAL_MAPBOX_PROTOZERO "Use a external mapnik/protozero. If off, use the submodule" OFF)
option(USE_EXTERNAL_MAPBOX_VARIANT "Use a external mapnik/variant. If off, use the submodule" OFF)
option(USE_JPEG "Adds jpeg support" ON)
option(USE_PNG "Adds png support" ON)
option(USE_TIFF "Adds tiff support" ON)
option(USE_WEBP "Adds webp support" ON)
option(USE_LIBXML2 "Adds libxml2 support" ON)
option(USE_CAIRO "Adds the cairo renderer" ON)

option(USE_PLUGIN_INPUT_CSV "adds plugin input csv" ON)
option(USE_PLUGIN_INPUT_GDAL "adds plugin input gdal" ON)
option(USE_PLUGIN_INPUT_GEOBUF "adds plugin input geobuf" ON)
option(USE_PLUGIN_INPUT_GEOJSON "adds plugin input geojson" ON)
option(USE_PLUGIN_INPUT_OGR "adds plugin input ogr" ON)
option(USE_PLUGIN_INPUT_PGRASTER "adds plugin input pgraster" ON)
option(USE_PLUGIN_INPUT_POSTGIS "adds plugin input postgis" ON)
option(USE_PLUGIN_INPUT_RASTER "adds plugin input raster" ON)
option(USE_PLUGIN_INPUT_SHAPE "adds plugin input shape" ON)
option(USE_PLUGIN_INPUT_SQLITE "adds plugin input sqlite" ON)
option(USE_PLUGIN_INPUT_TOPOJSON "adds plugin input topojson" ON)

option(BUILD_DEMO_VIEWER "builds the demo viewer" ON)
option(BUILD_DEMO_CPP "builds the demo c++ application" ON)



set(CXX_STANDARD 17)

find_package(Boost 1.74 REQUIRED COMPONENTS filesystem system regex program_options)
find_package(ICU REQUIRED COMPONENTS uc)
find_package(Freetype REQUIRED)
find_package(harfbuzz CONFIG REQUIRED)

if(USE_EXTERNAL_MAPBOX_GEOMETRY)
    # provide a way to specify the include dirs with the CACHE VARIABLES
    if(NOT MAPBOX_GEOMETRY_INCLUDE_DIRS)
        message(STATUS "Searching for the include dir of mapbox/geometry.hpp")
        find_path(MAPBOX_GEOMETRY_INCLUDE_DIRS "mapbox/geometry.hpp" REQUIRED)
    endif()
else()
    set(MAPBOX_GEOMETRY_INCLUDE_DIRS 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps/mapbox/geometry/include>
        $<INSTALL_INTERFACE:deps/mapbox/geometry/include>
    )
endif()
if(NOT MAPBOX_GEOMETRY_INCLUDE_DIRS)
    message(FATAL_ERROR "Set -DMAPBOX_GEOMETRY_INCLUDE_DIRS to the mapbox/geometry.hpp include dir") 
endif()

if(USE_EXTERNAL_MAPBOX_POLYLABEL)
    if(NOT MAPBOX_POLYLABEL_INCLUDE_DIRS)
        message(STATUS "Searching for the include dir of mapbox/polylabel")
        find_path(MAPBOX_POLYLABEL_INCLUDE_DIRS "mapbox/polylabel.hpp")
    endif()
else()
    set(MAPBOX_POLYLABEL_INCLUDE_DIRS 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps/mapbox/polylabel/include>
        $<INSTALL_INTERFACE:deps/mapbox/polylabel/include>
    )
endif()
if(NOT MAPBOX_POLYLABEL_INCLUDE_DIRS)
    message(FATAL_ERROR "Set MAPBOX_POLYLABEL_INCLUDE_DIRS to the mapbox/geometry include dir") 
endif()

if(USE_EXTERNAL_MAPBOX_PROTOZERO)
    if(NOT MAPBOX_PROTOZERO_INCLUDE_DIRS)
        message(STATUS "Searching for the include dir of mapbox/protozero")
        find_path(MAPBOX_PROTOZERO_INCLUDE_DIRS "protozero/pbf_message.hpp")
    endif()
else()
    set(MAPBOX_PROTOZERO_INCLUDE_DIRS 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps/mapbox/protozero/include>
        $<INSTALL_INTERFACE:deps/mapbox/protozero/include>
    )
endif()
if(NOT MAPBOX_PROTOZERO_INCLUDE_DIRS)
    message(FATAL_ERROR "Set MAPBOX_PROTOZERO_INCLUDE_DIRS to the mapbox/protozero include dir") 
endif()

if(USE_EXTERNAL_MAPBOX_VARIANT)
    if(NOT MAPBOX_VARIANT_INCLUDE_DIRS)
        message(STATUS "Searching for the include dir of mapbox/variant")
        find_path(MAPBOX_VARIANT_INCLUDE_DIRS "mapbox/variant.hpp")
    endif()
else()
    set(MAPBOX_VARIANT_INCLUDE_DIRS 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps/mapbox/variant/include>
        $<INSTALL_INTERFACE:deps/mapbox/variant/include>
    )
endif()
if(NOT MAPBOX_VARIANT_INCLUDE_DIRS)
    message(FATAL_ERROR "Set MAPBOX_VARIANT_INCLUDE_DIRS to the mapbox/variant include dir") 
endif()

set(MAPNIK_COMPILE_DEFS "")
set(MAPNIK_OPTIONAL_LIBS "")

if(USE_LIBXML2)
    message(STATUS "Using LibXml2")
    find_package(LibXml2 REQUIRED)
    list(APPEND MAPNIK_COMPILE_DEFS HAVE_LIBXML2)
    list(APPEND MAPNIK_OPTIONAL_LIBS LibXml2::LibXml2)
endif()
if(USE_PNG)
    message(STATUS "Using PNG")
    find_package(PNG REQUIRED)
    list(APPEND MAPNIK_COMPILE_DEFS HAVE_PNG)
    list(APPEND MAPNIK_OPTIONAL_LIBS PNG::PNG)
endif()

if(USE_JPEG)
    message(STATUS "Using JPEG")
    find_package(JPEG REQUIRED)
    list(APPEND MAPNIK_COMPILE_DEFS HAVE_JPEG)
    list(APPEND MAPNIK_OPTIONAL_LIBS JPEG::JPEG)
endif()

if(USE_TIFF)
    find_package(TIFF REQUIRED)
    message(STATUS "Using TIFF")
    list(APPEND MAPNIK_COMPILE_DEFS HAVE_TIFF)
    list(APPEND MAPNIK_OPTIONAL_LIBS TIFF::TIFF)
endif()

if(USE_WEBP)
    message(STATUS "Using WEBP")
    find_package(WebP CONFIG REQUIRED)
    list(APPEND MAPNIK_COMPILE_DEFS HAVE_WEBP)
    list(APPEND MAPNIK_OPTIONAL_LIBS WebP::webp)
endif()

if(USE_CAIRO)
    message(STATUS "Using Cairo renderer")
    find_package(Cairo REQUIRED)
    list(APPEND MAPNIK_COMPILE_DEFS HAVE_CAIRO)
    list(APPEND MAPNIK_OPTIONAL_LIBS Cairo::Cairo)
endif()

add_library(headers INTERFACE)
add_library(mapnik::headers ALIAS headers)

# todo: mapbox includes need to included private.
target_include_directories(headers INTERFACE  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  
    $<INSTALL_INTERFACE:include>
    ${MAPBOX_GEOMETRY_INCLUDE_DIRS}
    ${MAPBOX_POLYLABEL_INCLUDE_DIRS}
    ${MAPBOX_VARIANT_INCLUDE_DIRS}
    ${MAPBOX_PROTOZERO_INCLUDE_DIRS}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps>  
    $<INSTALL_INTERFACE:deps>
)
target_link_libraries(headers INTERFACE 
    Boost::boost
    Boost::regex
    Boost::filesystem
    ICU::uc
    Freetype::Freetype
    harfbuzz::harfbuzz
    ${MAPNIK_OPTIONAL_LIBS}
)
target_compile_definitions(headers INTERFACE ${MAPNIK_COMPILE_DEFS})

install(TARGETS headers 
    EXPORT MapnikTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include/
    PUBLIC_HEADER DESTINATION include/
)

add_subdirectory(deps)
add_subdirectory(src)
add_subdirectory(plugins)
add_subdirectory(utils)
add_subdirectory(demo)


include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/MapnikConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/MapnikConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/MapnikConfig.cmake"
  INSTALL_DESTINATION lib/cmake/mapnik
)

install(FILES
          "${CMAKE_CURRENT_BINARY_DIR}/MapnikConfig.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/MapnikConfigVersion.cmake"
        DESTINATION lib/cmake/mapnik
)
install(EXPORT MapnikTargets
        FILE MapnikTargets.cmake
        NAMESPACE mapnik::
        DESTINATION lib/cmake/mapnik
)

install(DIRECTORY include/ TYPE INCLUDE)
install(DIRECTORY deps/agg/include/ TYPE INCLUDE)
install(DIRECTORY fonts/ DESTINATION bin/fonts FILES_MATCHING PATTERN "*.py" EXCLUDE PATTERN "*")
