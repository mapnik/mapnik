find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS program_options)

add_executable(svg2png
    svg2png.cpp
)

target_link_libraries(svg2png PRIVATE
    Boost::program_options
    mapnik::mapnik
    mapnik::agg
)

set_target_properties(svg2png PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<BOOL:${BUILD_SHARED_CRT}>:DLL>"
)

mapnik_install_utility(svg2png)
