project(input-shape)

add_library(input-shape MODULE 
    dbfile.cpp 
    dbf_test.cpp
    shape_datasource.cpp
    shape_featureset.cpp
    shape_index_featureset.cpp
    shape_io.cpp shape_utils.cpp
)
add_library(mapnik::plugin::input::shape ALIAS input-shape)

target_link_libraries(input-shape PRIVATE mapnik::mapnik)

set_target_properties(input-shape PROPERTIES PREFIX "")
set_target_properties(input-shape PROPERTIES OUTPUT_NAME "shape")
set_target_properties(input-shape PROPERTIES SUFFIX ".input")

mapnik_install(TARGET input-shape IS_PLUGIN)
