find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS program_options)

add_executable(mapnik-render mapnik-render.cpp)

target_link_libraries(mapnik-render PRIVATE
    mapnik::mapnik
    Boost::program_options
    ICU::data ICU::i18n ICU::uc # needed for the static build (TODO: why isn't this correctly propagated from mapnik::mapnik?)
)

mapnik_install_utility(mapnik-render)
