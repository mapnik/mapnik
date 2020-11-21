cmake_minimum_required(VERSION 3.15.0)

add_library(json STATIC
    extract_bounding_boxes_x3.cpp
    feature_from_geojson.cpp
    feature_grammar_x3.cpp
    generic_json_grammar_x3.cpp
    geojson_grammar_x3.cpp
    geometry_from_geojson.cpp
    mapnik_feature_to_geojson.cpp
    mapnik_geometry_to_geojson.cpp
    mapnik_json_generator_grammar.cpp
    parse_feature.cpp
    positions_grammar_x3.cpp
    topojson_grammar_x3.cpp
    unicode_string_grammar_x3.cpp
)
add_library(mapnik::json ALIAS json)

target_include_directories(json PRIVATE
    ${JPEG_INCLUDE_DIR}
    ${LIBXML2_INCLUDE_DIR}
    ${MAPBOX_POLYLABEL_INCLUDE_DIRS} 
    ${MAPBOX_GEOMETRY_INCLUDE_DIRS}
    ${MAPBOX_VARIANT_INCLUDE_DIRS}
)
target_link_libraries(json PRIVATE mapnik::headers ${ICUU_LIB})

install(TARGETS json
    EXPORT MapnikTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)