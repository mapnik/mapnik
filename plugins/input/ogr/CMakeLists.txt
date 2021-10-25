project(input-ogr)

find_package(GDAL REQUIRED)

add_library(input-ogr MODULE 
    ogr_converter.cpp
    ogr_datasource.cpp
    ogr_featureset.cpp
    ogr_index_featureset.cpp
)
target_include_directories(input-ogr PRIVATE ${GDAL_INCLUDE_DIRS})
target_link_libraries(input-ogr PRIVATE
    mapnik::mapnik
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
