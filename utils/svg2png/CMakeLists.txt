find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS program_options)

add_executable(svg2png
    svg2png.cpp
)

target_link_libraries(svg2png PRIVATE
    Boost::program_options
    mapnik::mapnik
    mapnik::agg
    ICU::data ICU::i18n ICU::uc # needed for the static build (TODO: why isn't this correctly propagated from mapnik::mapnik?)
)

mapnik_install_utility(svg2png)
