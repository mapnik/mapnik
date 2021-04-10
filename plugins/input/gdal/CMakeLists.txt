project(input-gdal)

find_package(GDAL REQUIRED)

add_library(input-gdal SHARED
    gdal_datasource.cpp
    gdal_featureset.cpp
)
add_library(mapnik::plugin::input::gdal ALIAS input-gdal)

target_include_directories(input-gdal PRIVATE ${GDAL_INCLUDE_DIR})
target_link_libraries(input-gdal PRIVATE 
    mapnik::core 
    mapnik::mapnik
    ${GDAL_LIBRARY}
)
set_target_properties(input-gdal PROPERTIES PREFIX "")
set_target_properties(input-gdal PROPERTIES OUTPUT_NAME "gdal")
set_target_properties(input-gdal PROPERTIES SUFFIX ".input")

mapnik_install(TARGET input-gdal IS_PLUGIN)
