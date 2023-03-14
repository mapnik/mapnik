add_plugin_target(input-geojson "geojson")
target_sources(input-geojson ${_plugin_visibility}
    geojson_datasource.cpp
    geojson_featureset.cpp
    geojson_index_featureset.cpp
    geojson_memory_index_featureset.cpp
)
target_link_libraries(input-geojson ${_plugin_visibility}
    mapnik::mapnik
    mapnik::json
)
