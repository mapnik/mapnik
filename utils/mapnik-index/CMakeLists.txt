project(mapnik-index)

find_package(Boost 1.74 REQUIRED COMPONENTS program_options)

add_executable(mapnik-index
    mapnik-index.cpp
    process_csv_file.cpp
    process_geojson_file_x3.cpp
    ../../plugins/input/csv/csv_utils.cpp # this project depends on this file
)
target_link_libraries(mapnik-index PRIVATE
    mapnik::headers
    mapnik::mapnik
    mapnik::json
    mapnik::wkt
    Boost::program_options
)
install(TARGETS mapnik-index
    EXPORT MapnikTargets
    LIBRARY DESTINATION ${MAPNIK_LIB_DIR}
    ARCHIVE DESTINATION ${MAPNIK_ARCHIVE_DIR}
    RUNTIME DESTINATION ${MAPNIK_BIN_DIR}
)