add_executable(mapnik-index
    mapnik-index.cpp
    process_csv_file.cpp
    process_geojson_file_x3.cpp
)
target_link_libraries(mapnik-index PRIVATE mapnik::mapnik)
install(TARGETS mapnik-index
    EXPORT MapnikTargets
    RUNTIME DESTINATION bin
)