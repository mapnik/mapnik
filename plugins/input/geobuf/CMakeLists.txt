cmake_minimum_required(VERSION 3.15.0)
project(input-geobuf)

add_library(input-geobuf SHARED
    geobuf_datasource.cpp
    geobuf_featureset.cpp
)
add_library(mapnik::plugin::input::geobuf ALIAS input-geobuf)

target_link_libraries(input-geobuf PRIVATE  
    mapnik::headers 
    mapnik::mapnik
)
set_target_properties(input-geobuf PROPERTIES PREFIX "")
set_target_properties(input-geobuf PROPERTIES OUTPUT_NAME "geobuf")
set_target_properties(input-geobuf PROPERTIES SUFFIX ".input")

install(TARGETS input-geobuf 
    EXPORT MapnikTargets
    LIBRARY DESTINATION lib/plugins/input
    ARCHIVE DESTINATION lib/plugins/input
    RUNTIME DESTINATION bin/plugins/input
    INCLUDES DESTINATION .
    PUBLIC_HEADER DESTINATION .
)
