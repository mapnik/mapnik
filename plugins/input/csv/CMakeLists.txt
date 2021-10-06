project(input-csv)

add_library(input-csv MODULE
    csv_datasource.cpp
    csv_featureset.cpp
    csv_index_featureset.cpp
    csv_inline_featureset.cpp
    csv_utils.cpp
)
target_link_libraries(input-csv PRIVATE
    mapnik::mapnik 
    mapnik::wkt 
    mapnik::json
)
set_target_properties(input-csv PROPERTIES 
    OUTPUT_NAME "csv"
    PREFIX "${_plugin_prefix}"
    SUFFIX "${_plugin_suffix}"
)

mapnik_install_plugin(input-csv)
