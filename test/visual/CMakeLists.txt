add_executable(mapnik-test-visual
    parse_map_sizes.cpp
    report.cpp
    runner.cpp
    run.cpp
)
target_link_libraries(mapnik-test-visual PRIVATE
    Catch2::Catch2
    Boost::program_options
    mapnik::mapnik
    mapnik::agg
    ICU::data ICU::i18n ICU::uc # needed for the static build (TODO: why isn't this correctly propagated from mapnik::mapnik?)
)

if(CMAKE_CXX_STANDARD LESS 17 OR USE_BOOST_FILESYSTEM)
target_link_libraries(mapnik-test-visual PRIVATE
    Boost::filesystem
)
endif()

# needed for cleanup.hpp
target_include_directories(mapnik-test-visual PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/..")

mapnik_install_utility(mapnik-test-visual)
