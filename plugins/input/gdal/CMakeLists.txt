find_package(GDAL REQUIRED)

add_plugin_target(input-gdal "gdal")
target_sources(input-gdal ${_plugin_visibility}
    gdal_datasource.cpp
    gdal_featureset.cpp
)
target_include_directories(input-gdal ${_plugin_visibility} ${GDAL_INCLUDE_DIRS})
target_link_libraries(input-gdal ${_plugin_visibility}
    mapnik::mapnik
    ${GDAL_LIBRARIES}
)
