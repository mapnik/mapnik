project(input-topojson)

add_library(input-topojson SHARED
    topojson_datasource.cpp
    topojson_featureset.cpp
)
add_library(mapnik::plugin::input::topojson ALIAS input-topojson)

target_link_libraries(input-topojson PRIVATE 
    mapnik::headers 
    mapnik::mapnik
    mapnik::json
)

set_target_properties(input-topojson PROPERTIES PREFIX "")
set_target_properties(input-topojson PROPERTIES OUTPUT_NAME "topojson")
set_target_properties(input-topojson PROPERTIES SUFFIX ".input")

install(TARGETS input-topojson 
    RUNTIME DESTINATION ${PLUGINS_INSTALL_DIR}
)
