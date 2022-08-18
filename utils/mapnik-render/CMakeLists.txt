find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS program_options)

add_executable(mapnik-render mapnik-render.cpp)

target_link_libraries(mapnik-render PRIVATE
    mapnik::mapnik
    Boost::program_options
)

mapnik_install_utility(mapnik-render)
