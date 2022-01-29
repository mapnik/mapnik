find_package(GDAL REQUIRED)

add_library(input-ogr ${_plugin_linkage})
target_sources(input-ogr ${_plugin_visibility}
    ogr_converter.cpp
    ogr_datasource.cpp
    ogr_featureset.cpp
    ogr_index_featureset.cpp
)
target_include_directories(input-ogr ${_plugin_visibility} ${GDAL_INCLUDE_DIRS})
target_link_libraries(input-ogr ${_plugin_visibility}
    mapnik::mapnik
    mapnik::datasource-base
    ${GDAL_LIBRARIES}
)
set_target_properties(input-ogr PROPERTIES 
    OUTPUT_NAME "ogr"
    PREFIX "${_plugin_prefix}"
    SUFFIX "${_plugin_suffix}"
    LIBRARY_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/plugins/input"
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/lib"
)
mapnik_install_plugin(input-ogr)
