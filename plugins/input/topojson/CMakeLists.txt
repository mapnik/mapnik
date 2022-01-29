add_library(input-topojson ${_plugin_linkage})
target_sources(input-topojson ${_plugin_visibility}
    topojson_datasource.cpp
    topojson_featureset.cpp
)
target_link_libraries(input-topojson ${_plugin_visibility}
    mapnik::mapnik
    mapnik::json
    mapnik::datasource-base
)
set_target_properties(input-topojson PROPERTIES 
    OUTPUT_NAME "topojson"
    PREFIX "${_plugin_prefix}"
    SUFFIX "${_plugin_suffix}"
    LIBRARY_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/plugins/input"
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/lib"
)
mapnik_install_plugin(input-topojson)
