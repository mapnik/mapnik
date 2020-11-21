cmake_minimum_required(VERSION 3.15.0)
project(input-gdal)

find_package(GDAL REQUIRED)

add_library(input-gdal SHARED
    gdal_datasource.cpp
    gdal_featureset.cpp
)
add_library(mapnik::plugin::input::gdal ALIAS input-gdal)

target_include_directories(input-gdal PRIVATE ${GDAL_INCLUDE_DIR})
target_link_libraries(input-gdal PRIVATE 
    mapnik::headers 
    mapnik::mapnik
    ${GDAL_LIBRARY}
)
set_target_properties(input-gdal PROPERTIES PREFIX "")
set_target_properties(input-gdal PROPERTIES OUTPUT_NAME "gdal")
set_target_properties(input-gdal PROPERTIES SUFFIX ".input")

install(TARGETS input-gdal 
    EXPORT MapnikTargets
    LIBRARY DESTINATION lib/plugins/input
    ARCHIVE DESTINATION lib/plugins/input
    RUNTIME DESTINATION bin/plugins/input
    INCLUDES DESTINATION .
    PUBLIC_HEADER DESTINATION .
)
