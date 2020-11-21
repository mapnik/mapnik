cmake_minimum_required(VERSION 3.15.0)
project(input-csv)

add_library(input-csv SHARED
    csv_datasource.cpp
    csv_featureset.cpp
    csv_index_featureset.cpp
    csv_inline_featureset.cpp
    csv_utils.cpp
)
add_library(mapnik::plugin::input::csv ALIAS input-csv)

target_link_libraries(input-csv PRIVATE 
    mapnik::headers 
    mapnik::mapnik 
    mapnik::wkt 
    mapnik::json
)
set_target_properties(input-csv PROPERTIES PREFIX "")
set_target_properties(input-csv PROPERTIES OUTPUT_NAME "csv")
set_target_properties(input-csv PROPERTIES SUFFIX ".input")
#install(TARGETS input-csv DESTINATION bin/plugins/input)

install(TARGETS input-csv 
    EXPORT MapnikTargets
    LIBRARY DESTINATION lib/plugins/input
    ARCHIVE DESTINATION lib/plugins/input
    RUNTIME DESTINATION bin/plugins/input
    INCLUDES DESTINATION .
    PUBLIC_HEADER DESTINATION .
)