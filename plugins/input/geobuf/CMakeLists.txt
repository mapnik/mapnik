add_library(input-geobuf MODULE
    geobuf_datasource.cpp
    geobuf_featureset.cpp
)
target_link_libraries(input-geobuf PRIVATE mapnik::mapnik)
set_target_properties(input-geobuf PROPERTIES 
    OUTPUT_NAME "geobuf"
    PREFIX "${_plugin_prefix}"
    SUFFIX "${_plugin_suffix}"
    LIBRARY_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/plugins/input"
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/lib"
)
mapnik_install_plugin(input-geobuf)
