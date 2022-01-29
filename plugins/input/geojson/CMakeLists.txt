add_library(input-geojson ${_plugin_linkage})
target_sources(input-geojson ${_plugin_visibility}
    geojson_datasource.cpp 
    geojson_featureset.cpp 
    geojson_index_featureset.cpp 
    geojson_memory_index_featureset.cpp
)
target_link_libraries(input-geojson ${_plugin_visibility}
    mapnik::mapnik
    mapnik::json
    mapnik::datasource-base
)
set_target_properties(input-geojson PROPERTIES 
    OUTPUT_NAME "geojson"
    PREFIX "${_plugin_prefix}"
    SUFFIX "${_plugin_suffix}"
    LIBRARY_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/plugins/input"
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/lib"
)
mapnik_install_plugin(input-geojson)
