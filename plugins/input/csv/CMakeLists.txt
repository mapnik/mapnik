project(input-csv)

add_library(input-csv MODULE
    csv_datasource.cpp
    csv_featureset.cpp
    csv_index_featureset.cpp
    csv_inline_featureset.cpp
    csv_utils.cpp
)
add_library(mapnik::plugin::input::csv ALIAS input-csv)

target_link_libraries(input-csv PRIVATE 
    mapnik::core 
    mapnik::mapnik 
    mapnik::wkt 
    mapnik::json
)
set_target_properties(input-csv PROPERTIES PREFIX "")
set_target_properties(input-csv PROPERTIES OUTPUT_NAME "csv")
set_target_properties(input-csv PROPERTIES SUFFIX ".input")

mapnik_install(TARGET input-csv IS_PLUGIN)
