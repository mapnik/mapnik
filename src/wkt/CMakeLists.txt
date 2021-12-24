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
    POSITION_INDEPENDENT_CODE ON
    DEBUG_POSTFIX "${MAPNIK_DEBUG_POSTFIX}"
    OUTPUT_NAME "mapnikwkt"
    PREFIX "lib"
    IMPORT_PREFIX "lib"
    VERSION ${MAPNIK_VERSION} 
    # see mapnik target for explanation
    SOVERSION ${MAPNIK_VERSION}
    LIBRARY_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/lib"
)

mapnik_install(wkt)
