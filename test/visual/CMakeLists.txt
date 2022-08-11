add_executable(mapnik-test-visual
    parse_map_sizes.cpp
    report.cpp
    runner.cpp
    run.cpp
)
set_target_properties(mapnik-test-visual PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/lib"
)
target_link_libraries(mapnik-test-visual PRIVATE
    Catch2::Catch2
    Boost::program_options
    Boost::filesystem
    mapnik::mapnik
    mapnik::agg
)
# needed for cleanup.hpp
target_include_directories(mapnik-test-visual PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/..")

mapnik_install_utility(mapnik-test-visual)
