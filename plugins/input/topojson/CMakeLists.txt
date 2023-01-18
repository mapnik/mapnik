add_plugin_target(input-topojson "topojson")
target_sources(input-topojson ${_plugin_visibility}
    topojson_datasource.cpp
    topojson_featureset.cpp
)
target_link_libraries(input-topojson ${_plugin_visibility}
    mapnik::mapnik
    mapnik::json
    mapnik::datasource-base
)
