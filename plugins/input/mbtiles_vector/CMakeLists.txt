# SPDX-License-Identifier:  	LGPL-2.1-or-later
add_library(input-mbtiles_vector MODULE)
set_target_properties(input-mbtiles_vector PROPERTIES
    OUTPUT_NAME mbtiles_vector
    POSITION_INDEPENDENT_CODE ON
    PREFIX ""
    SUFFIX ".input"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
install(TARGETS input-mbtiles_vector
    LIBRARY DESTINATION ${MAPNIK_PLUGIN_INSTALL_DIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
target_sources(input-mbtiles_vector PRIVATE
    mbtiles_vector_datasource.cpp
    mbtiles_vector_featureset.cpp
    mvt_io.cpp
    vector_tile_compression.cpp
    vector_tile_geometry_decoder.cpp
)
target_include_directories(input-mbtiles_vector PRIVATE
    ${MAPNIK_INCLUDE_DIRS}
    ${MAPNIK_DEPS_INCLUDE_DIRS}
    ${PROTOZERO_INCLUDE_DIR}
)
target_link_libraries(input-mbtiles_vector PRIVATE
    ${MAPNIK_LIBRARIES}
    SQLite::SQLite3
    ZLIB::ZLIB
)
