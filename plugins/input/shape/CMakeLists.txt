add_plugin_target(input-shape "shape")
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
