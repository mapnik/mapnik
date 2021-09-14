project(mapnik-index)

find_package(Boost REQUIRED COMPONENTS program_options)

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

mapnik_install(TARGET mapnik-index)
