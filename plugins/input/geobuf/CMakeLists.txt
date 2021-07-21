project(input-geobuf)

add_library(input-geobuf MODULE
    geobuf_datasource.cpp
    geobuf_featureset.cpp
)
add_library(mapnik::plugin::input::geobuf ALIAS input-geobuf)

target_link_libraries(input-geobuf PRIVATE  
    mapnik::core 
    mapnik::mapnik
)
set_target_properties(input-geobuf PROPERTIES PREFIX "")
set_target_properties(input-geobuf PROPERTIES OUTPUT_NAME "geobuf")
set_target_properties(input-geobuf PROPERTIES SUFFIX ".input")

mapnik_install(TARGET input-geobuf IS_PLUGIN)
