target_sources(json PRIVATE
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

target_include_directories(json PRIVATE
    ${JPEG_INCLUDE_DIR}
    ${LIBXML2_INCLUDE_DIR}
    ${MAPBOX_POLYLABEL_INCLUDE_DIRS}
    ${MAPBOX_GEOMETRY_INCLUDE_DIRS}
    ${MAPBOX_VARIANT_INCLUDE_DIRS}
)
target_link_libraries(json PRIVATE mapnik::core ${ICUU_LIB})

set_target_properties(json PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME "mapnikjson"
    PREFIX "lib"
    IMPORT_PREFIX "lib"
    VERSION ${MAPNIK_VERSION}
    # see mapnik target for explanation
    SOVERSION ${MAPNIK_MAJOR_VERSION}.${MAPNIK_MINOR_VERSION}
)

mapnik_install(json)
