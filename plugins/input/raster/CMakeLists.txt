add_plugin_target(input-raster "raster")
target_sources(input-raster ${_plugin_visibility}
    raster_datasource.cpp
    raster_featureset.cpp
    raster_info.cpp
)
target_link_libraries(input-raster ${_plugin_visibility}
    mapnik::mapnik
    mapnik::datasource-base
)
