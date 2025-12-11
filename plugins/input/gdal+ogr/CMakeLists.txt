find_package(GDAL REQUIRED)

add_plugin_target(input-gdal+ogr "gdal+ogr")
target_sources(input-gdal+ogr ${_plugin_visibility}
    gdal_datasource.cpp
    gdal_featureset.cpp
    ogr_converter.cpp
    ogr_datasource.cpp
    ogr_utils.cpp
    ogr_featureset.cpp
    ogr_index_featureset.cpp
)
target_include_directories(input-gdal+ogr ${_plugin_visibility} ${GDAL_INCLUDE_DIRS})
target_link_libraries(input-gdal+ogr ${_plugin_visibility}
    mapnik::mapnik
    SQLite::SQLite3
    "/Users/artemp/projects/mapnik-consulting/mapnik-package-builder/_deps/lib/libgeos.a"
    "/Users/artemp/projects/mapnik-consulting/mapnik-package-builder/_deps/lib/libavif.a"
    ${GDAL_LIBRARIES}
)
