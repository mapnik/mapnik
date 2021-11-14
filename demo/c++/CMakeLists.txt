add_executable(mapnik-demo rundemo.cpp)
set_target_properties(mapnik-demo PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/lib"
)

target_link_libraries(mapnik-demo PRIVATE mapnik::agg mapnik::mapnik)
