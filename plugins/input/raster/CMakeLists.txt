add_library(input-raster MODULE
    raster_datasource.cpp
    raster_featureset.cpp
    raster_info.cpp
)
target_link_libraries(input-raster PRIVATE mapnik::mapnik)
set_target_properties(input-raster PROPERTIES
    OUTPUT_NAME "raster"
    PREFIX "${_plugin_prefix}"
    SUFFIX "${_plugin_suffix}"
    LIBRARY_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/plugins/input"
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/lib"
)
mapnik_install_plugin(input-raster)
