find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS program_options)

add_executable(shapeindex
    shapeindex.cpp
)

target_include_directories(shapeindex PRIVATE ../../plugins/input/shape)
target_link_libraries(shapeindex PRIVATE
    Boost::program_options
    mapnik::mapnik
    ICU::data ICU::i18n ICU::uc # needed for the static build (TODO: why isn't this correctly propagated from mapnik::mapnik?)
)

mapnik_install_utility(shapeindex)
