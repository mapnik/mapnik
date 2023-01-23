add_plugin_target(input-csv "csv")

target_sources(input-csv ${_plugin_visibility}
    csv_datasource.cpp
    csv_featureset.cpp
    csv_index_featureset.cpp
    csv_inline_featureset.cpp
    csv_utils.cpp
)
target_link_libraries(input-csv ${_plugin_visibility}
    mapnik::mapnik
    mapnik::wkt
    mapnik::json
)
