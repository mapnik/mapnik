cmake_minimum_required(VERSION 3.15.0)
project(input-ogr)

find_package(GDAL REQUIRED)

add_library(input-ogr SHARED 
    ogr_converter.cpp
    ogr_datasource.cpp
    ogr_featureset.cpp
    ogr_index_featureset.cpp
)
add_library(mapnik::plugin::input::ogr ALIAS input-ogr)

target_include_directories(input-ogr PRIVATE ${GDAL_INCLUDE_DIR})
target_link_libraries(input-ogr PRIVATE 
    mapnik::headers 
    mapnik::mapnik
    ${GDAL_LIBRARY}
)
set_target_properties(input-ogr PROPERTIES PREFIX "")
set_target_properties(input-ogr PROPERTIES OUTPUT_NAME "ogr")
set_target_properties(input-ogr PROPERTIES SUFFIX ".input")

install(TARGETS input-ogr 
    EXPORT MapnikTargets
    LIBRARY DESTINATION lib/plugins/input
    ARCHIVE DESTINATION lib/plugins/input
    RUNTIME DESTINATION bin/plugins/input
    INCLUDES DESTINATION .
    PUBLIC_HEADER DESTINATION .
)
