add_plugin_target(input-geobuf "geobuf")

target_sources(input-geobuf ${_plugin_visibility}
    geobuf_datasource.cpp
    geobuf_featureset.cpp
)
target_link_libraries(input-geobuf ${_plugin_visibility} 
    mapnik::mapnik
    mapnik::datasource-base
)
