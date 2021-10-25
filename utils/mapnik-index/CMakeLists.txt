project(mapnik-index)

find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS program_options)

add_executable(mapnik-index
    mapnik-index.cpp
    process_csv_file.cpp
    process_geojson_file_x3.cpp
    ../../plugins/input/csv/csv_utils.cpp # this project depends on this file
)
target_link_libraries(mapnik-index PRIVATE
    mapnik::mapnik
    mapnik::json
    mapnik::wkt
    Boost::program_options
)

set_target_properties(mapnik-index PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
)

mapnik_install_utility(mapnik-index)
