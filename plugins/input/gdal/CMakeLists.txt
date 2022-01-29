find_package(GDAL REQUIRED)

add_library(input-gdal ${_plugin_linkage})
target_sources(input-gdal ${_plugin_visibility}
    gdal_datasource.cpp
    gdal_featureset.cpp
)
target_include_directories(input-gdal ${_plugin_visibility} ${GDAL_INCLUDE_DIRS})
target_link_libraries(input-gdal ${_plugin_visibility} 
    mapnik::mapnik
    mapnik::datasource-base
    ${GDAL_LIBRARIES}
)
set_target_properties(input-gdal PROPERTIES 
    OUTPUT_NAME "gdal"
    PREFIX "${_plugin_prefix}"
    SUFFIX "${_plugin_suffix}"
    LIBRARY_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/plugins/input"
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/lib"
)
mapnik_install_plugin(input-gdal)
