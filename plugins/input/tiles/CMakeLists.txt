mapnik_find_package(SQLite3 REQUIRED)
mapnik_find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS json)

add_plugin_target(input-tiles "tiles")
add_compile_definitions(MAPNIK_VECTOR_TILE_LIBRARY=1)

target_sources(input-tiles ${_plugin_visibility}
    tiles_datasource.cpp
    tiles_featureset.cpp
    mvt_io.cpp
    ${PROJECT_SOURCE_DIR}/deps/mapbox/mapnik-vector-tile/src/vector_tile_compression.cpp
    ${PROJECT_SOURCE_DIR}/deps/mapbox/mapnik-vector-tile/src/vector_tile_geometry_decoder.cpp
)

target_include_directories(input-tiles ${_plugin_visibility} "${PROJECT_SOURCE_DIR}/deps/mapbox/mapnik-vector-tile/src")
target_include_directories(input-tiles ${_plugin_visibility} "${PROJECT_SOURCE_DIR}/plugins/input/sqlite")

target_link_libraries(input-tiles ${_plugin_visibility}
    mapnik::mapnik
    SQLite::SQLite3
    Boost::json
)
