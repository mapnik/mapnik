cmake_minimum_required(VERSION 3.15.0)
project(mapnik 
    VERSION 3.0.24 
    HOMEPAGE_URL "https://mapnik.org/" 
    DESCRIPTION "Mapnik is an open source toolkit for developing mapping applications" 
    LANGUAGES CXX 
)

option(BUILD_SHARED_LIBS ON)
option(USE_EXTERNAL_MAPBOX_GEOMETRY OFF)
option(USE_EXTERNAL_MAPBOX_POLYLABEL OFF)
option(USE_EXTERNAL_MAPBOX_PROTOZERO OFF)
option(USE_EXTERNAL_MAPBOX_VARIANT OFF)

set(CXX_STANDARD 17)

find_package(Boost 1.74 REQUIRED COMPONENTS filesystem system regex program_options)
find_package(ICU REQUIRED COMPONENTS uc)

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
    ICU::uc
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
#add_subdirectory(utils)
#add_subdirectory(plugins)
