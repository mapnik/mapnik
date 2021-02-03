project(input-raster)

add_library(input-raster SHARED
    raster_datasource.cpp
    raster_featureset.cpp
    raster_info.cpp
)
add_library(mapnik::plugin::input::raster ALIAS input-raster)

target_link_libraries(input-raster PRIVATE 
    mapnik::headers 
    mapnik::mapnik
)

set_target_properties(input-raster PROPERTIES PREFIX "")
set_target_properties(input-raster PROPERTIES OUTPUT_NAME "raster")
set_target_properties(input-raster PROPERTIES SUFFIX ".input")

install(TARGETS input-raster 
    RUNTIME DESTINATION ${PLUGINS_INSTALL_DIR}
)
