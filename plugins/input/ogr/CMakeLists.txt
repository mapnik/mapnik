project(input-ogr)

find_package(GDAL REQUIRED)

add_library(input-ogr MODULE 
    ogr_converter.cpp
    ogr_datasource.cpp
    ogr_featureset.cpp
    ogr_index_featureset.cpp
)
add_library(mapnik::plugin::input::ogr ALIAS input-ogr)

target_include_directories(input-ogr PRIVATE ${GDAL_INCLUDE_DIRS})
target_link_libraries(input-ogr PRIVATE 
    mapnik::core 
    mapnik::mapnik
    ${GDAL_LIBRARIES}
)
set_target_properties(input-ogr PROPERTIES PREFIX "")
set_target_properties(input-ogr PROPERTIES OUTPUT_NAME "ogr")
set_target_properties(input-ogr PROPERTIES SUFFIX ".input")

mapnik_install(TARGET input-ogr IS_PLUGIN)
