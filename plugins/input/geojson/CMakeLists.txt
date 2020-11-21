cmake_minimum_required(VERSION 3.15.0)
project(input-geojson)

add_library(input-geojson SHARED 
    geojson_datasource.cpp 
    geojson_featureset.cpp 
    geojson_index_featureset.cpp 
    geojson_memory_index_featureset.cpp
)
add_library(mapnik::plugin::input::geojson ALIAS input-geojson)

target_link_libraries(input-geojson PRIVATE 
    mapnik::headers 
    mapnik::mapnik
    mapnik::json
)

set_target_properties(input-geojson PROPERTIES PREFIX "")
set_target_properties(input-geojson PROPERTIES OUTPUT_NAME "geojson")
set_target_properties(input-geojson PROPERTIES SUFFIX ".input")

install(TARGETS input-geojson 
    LIBRARY DESTINATION lib/plugins/input
    ARCHIVE DESTINATION lib/plugins/input
    RUNTIME DESTINATION bin/plugins/input
)
