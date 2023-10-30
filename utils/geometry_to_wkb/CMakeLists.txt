add_executable(geometry_to_wkb main.cpp)

target_link_libraries(geometry_to_wkb PRIVATE
    mapnik::mapnik
    ICU::data ICU::i18n ICU::uc # needed for the static build (TODO: why isn't this correctly propagated from mapnik::mapnik?)
)

mapnik_install_utility(geometry_to_wkb)
