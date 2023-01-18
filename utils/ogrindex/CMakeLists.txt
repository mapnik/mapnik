add_executable(ogrindex ogrindex.cpp)

target_link_libraries(ogrindex PRIVATE
    mapnik::mapnik
    ICU::data ICU::i18n ICU::uc # needed for the static build (TODO: why isn't this correctly propagated from mapnik::mapnik?)
)

mapnik_install_utility(ogrindex)
