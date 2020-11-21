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

set(CXX_STANDARD 17)

find_package(Boost 1.74 REQUIRED COMPONENTS filesystem system regex program_options)
find_package(ICU REQUIRED COMPONENTS uc)
find_package(Freetype REQUIRED)
find_package(harfbuzz CONFIG REQUIRED)

if(USE_EXTERNAL_MAPBOX_GEOMETRY)
    find_path(MAPBOX_GEOMETRY_INCLUDE_DIRS "mapbox/geometry.hpp")
else()
    set(MAPBOX_GEOMETRY_INCLUDE_DIRS deps/mapbox/geometry/include)
endif()
if(NOT MAPBOX_GEOMETRY_INCLUDE_DIRS)
    message(FATAL_ERROR "Set MAPBOX_GEOMETRY_INCLUDE_DIRS to the mapbox/geometry.hpp include dir") 
endif()

if(USE_EXTERNAL_MAPBOX_POLYLABEL)
    find_path(MAPBOX_POLYLABEL_INCLUDE_DIRS "mapbox/polylabel.hpp")
else()
    set(MAPBOX_POLYLABEL_INCLUDE_DIRS deps/mapbox/polylabel/include)
endif()
if(NOT MAPBOX_POLYLABEL_INCLUDE_DIRS)
    message(FATAL_ERROR "Set MAPBOX_POLYLABEL_INCLUDE_DIRS to the mapbox/geometry include dir") 
endif()

if(USE_EXTERNAL_MAPBOX_PROTOZERO)
    find_path(MAPBOX_PROTOZERO_INCLUDE_DIRS "protozero/pbf_message.hpp")
else()
    set(MAPBOX_PROTOZERO_INCLUDE_DIRS deps/mapbox/protozero/include)
endif()
if(NOT MAPBOX_PROTOZERO_INCLUDE_DIRS)
    message(FATAL_ERROR "Set MAPBOX_PROTOZERO_INCLUDE_DIRS to the mapbox/protozero include dir") 
endif()

if(USE_EXTERNAL_MAPBOX_VARIANT)
    find_path(MAPBOX_VARIANT_INCLUDE_DIRS "mapbox/variant.hpp")
else()
    set(MAPBOX_VARIANT_INCLUDE_DIRS deps/mapbox/variant/include)
endif()
if(NOT MAPBOX_VARIANT_INCLUDE_DIRS)
    message(FATAL_ERROR "Set MAPBOX_VARIANT_INCLUDE_DIRS to the mapbox/variant include dir") 
endif()

set(MAPNIK_COMPILE_DEFS "")
set(MAPNIK_OPTIONAL_LIBS "")

if(USE_LIBXML2)
    message(STATUS "Using LibXml2")
    find_package(LibXml2 REQUIRED)
    set(MAPNIK_COMPILE_DEFS ${MAPNIK_COMPILE_DEFS} HAVE_LIBXML2)
    set(MAPNIK_OPTIONAL_LIBS ${MAPNIK_OPTIONAL_LIBS} LibXml2::LibXml2)
endif()
if(USE_PNG)
    message(STATUS "Using PNG")
    find_package(PNG REQUIRED)
    set(MAPNIK_COMPILE_DEFS ${MAPNIK_COMPILE_DEFS} HAVE_PNG)
    set(MAPNIK_OPTIONAL_LIBS ${MAPNIK_OPTIONAL_LIBS} PNG::PNG)
endif()

if(USE_JPEG)
    message(STATUS "Using JPEG")
    find_package(JPEG REQUIRED)
    set(MAPNIK_COMPILE_DEFS ${MAPNIK_COMPILE_DEFS} HAVE_JPEG)
    set(MAPNIK_OPTIONAL_LIBS ${MAPNIK_OPTIONAL_LIBS} JPEG::JPEG)
endif()

if(USE_TIFF)
    find_package(TIFF REQUIRED)
    message(STATUS "Using TIFF")
    set(MAPNIK_COMPILE_DEFS ${MAPNIK_COMPILE_DEFS} HAVE_TIFF)
    set(MAPNIK_OPTIONAL_LIBS ${MAPNIK_OPTIONAL_LIBS} TIFF::TIFF)
endif()

if(USE_WEBP)
    message(STATUS "Using WEBP")
    find_package(WebP CONFIG REQUIRED)
    set(MAPNIK_COMPILE_DEFS ${MAPNIK_COMPILE_DEFS} HAVE_WEBP)
    set(MAPNIK_OPTIONAL_LIBS ${MAPNIK_OPTIONAL_LIBS} WebP::webp)
endif()

if(USE_CAIRO)
    message(STATUS "Using Cairo renderer")
    find_package(Cairo REQUIRED)
    set(MAPNIK_COMPILE_DEFS ${MAPNIK_COMPILE_DEFS} HAVE_CAIRO)
    set(MAPNIK_OPTIONAL_LIBS ${MAPNIK_OPTIONAL_LIBS} Cairo::Cairo)
endif()

add_library(mapnik-headers INTERFACE)
add_library(mapnik::headers ALIAS mapnik-headers)

target_include_directories(mapnik-headers INTERFACE  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  
    $<INSTALL_INTERFACE:include>
    ${MAPBOX_GEOMETRY_INCLUDE_DIRS}
    ${MAPBOX_POLYLABEL_INCLUDE_DIRS}
    ${MAPBOX_VARIANT_INCLUDE_DIRS}
    ${MAPBOX_PROTOZERO_INCLUDE_DIRS}
    deps
)
target_link_libraries(mapnik-headers INTERFACE 
    Boost::boost
    Boost::regex
    Boost::filesystem
    ICU::uc
    Freetype::Freetype
    harfbuzz::harfbuzz
    ${MAPNIK_OPTIONAL_LIBS}
)
if(BUILD_SHARED_LIBS)
    target_compile_definitions(mapnik-headers INTERFACE MAPNIK_EXPORTS)
endif()

install(TARGETS mapnik-headers 
    EXPORT MapnikTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include/
    PUBLIC_HEADER DESTINATION include/
)

add_subdirectory(deps)
add_subdirectory(src)
add_subdirectory(utils)
#add_subdirectory(utils)
#add_subdirectory(plugins)
