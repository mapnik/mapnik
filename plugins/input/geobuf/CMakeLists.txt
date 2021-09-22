project(input-geobuf)

add_library(input-geobuf MODULE
    geobuf_datasource.cpp
    geobuf_featureset.cpp
)
target_link_libraries(input-geobuf PRIVATE mapnik::mapnik)
set_target_properties(input-geobuf PROPERTIES 
    OUTPUT_NAME "geobuf"
    PREFIX "${_plugin_prefix}"
    SUFFIX "${_plugin_suffix}"
)

mapnik_install_plugin(input-geobuf)
