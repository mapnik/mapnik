add_library(input-raster ${_plugin_linkage})
target_sources(input-raster ${_plugin_visibility}
    raster_datasource.cpp
    raster_featureset.cpp
    raster_info.cpp
)
target_link_libraries(input-raster ${_plugin_visibility} 
    mapnik::mapnik
    mapnik::datasource-base
)
set_target_properties(input-raster PROPERTIES 
    OUTPUT_NAME "raster"
    PREFIX "${_plugin_prefix}"
    SUFFIX "${_plugin_suffix}"
    LIBRARY_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/plugins/input"
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/lib"
)
mapnik_install_plugin(input-raster)
