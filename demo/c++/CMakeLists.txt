add_executable(mapnik-demo rundemo.cpp)
target_link_libraries(mapnik-demo PRIVATE
    mapnik::agg
    mapnik::mapnik
    ICU::data ICU::i18n ICU::uc # needed for the static build (TODO: why isn't this correctly propagated from mapnik::mapnik?)
)
