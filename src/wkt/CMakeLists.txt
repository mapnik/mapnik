add_library(wkt STATIC
    geometry_to_wkt.cpp
    mapnik_wkt_generator_grammar.cpp
    wkt_factory.cpp
    wkt_grammar_x3.cpp
)
add_library(mapnik::wkt ALIAS wkt)

target_include_directories(wkt PRIVATE
    ${MAPNIK_INCLUDE_PATH}
    ${JPEG_INCLUDE_DIR}
    ${LIBXML2_INCLUDE_DIR}
    ${MAPBOX_POLYLABEL_INCLUDE_DIRS} 
    ${MAPBOX_GEOMETRY_INCLUDE_DIRS}
    ${MAPBOX_VARIANT_INCLUDE_DIRS}
)
target_link_libraries(wkt PRIVATE mapnik::core)

set_target_properties(wkt PROPERTIES 
    DEBUG_POSTFIX ${MAPNIK_DEBUG_POSTFIX}
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS wkt 
    EXPORT MapnikTargets
    LIBRARY DESTINATION ${MAPNIK_LIB_DIR}
    ARCHIVE DESTINATION ${MAPNIK_ARCHIVE_DIR}
    RUNTIME DESTINATION ${MAPNIK_BIN_DIR}
    INCLUDES DESTINATION include/
    PUBLIC_HEADER DESTINATION include/
    COMPONENT mapnik
)

mapnik_install(TARGET wkt)
