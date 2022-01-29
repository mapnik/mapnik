add_library(input-shape ${_plugin_linkage})
target_sources(input-shape ${_plugin_visibility}
    dbfile.cpp 
    dbf_test.cpp
    shape_datasource.cpp
    shape_featureset.cpp
    shape_index_featureset.cpp
    shape_io.cpp shape_utils.cpp
)
target_link_libraries(input-shape ${_plugin_visibility} 
    mapnik::mapnik
    mapnik::datasource-base
)
set_target_properties(input-shape PROPERTIES 
    OUTPUT_NAME "shape"
    PREFIX "${_plugin_prefix}"
    SUFFIX "${_plugin_suffix}"
    LIBRARY_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/plugins/input"
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/lib"
)
mapnik_install_plugin(input-shape)
