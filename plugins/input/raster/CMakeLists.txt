project(input-raster)

add_library(input-raster MODULE
    raster_datasource.cpp
    raster_featureset.cpp
    raster_info.cpp
)
add_library(mapnik::plugin::input::raster ALIAS input-raster)

target_link_libraries(input-raster PRIVATE mapnik::mapnik)

set_target_properties(input-raster PROPERTIES PREFIX "")
set_target_properties(input-raster PROPERTIES OUTPUT_NAME "raster")
set_target_properties(input-raster PROPERTIES SUFFIX ".input")

mapnik_install(TARGET input-raster IS_PLUGIN)
