project(input-pgraster)

find_package(PostgreSQL REQUIRED)

add_library(input-pgraster MODULE 
    pgraster_datasource.cpp 
    pgraster_featureset.cpp 
    pgraster_wkb_reader.cpp
)
add_library(mapnik::plugin::input::pgraster ALIAS input-pgraster)


target_link_libraries(input-pgraster PRIVATE 
    mapnik::core 
    mapnik::mapnik  
    PostgreSQL::PostgreSQL
)
set_target_properties(input-pgraster PROPERTIES PREFIX "")
set_target_properties(input-pgraster PROPERTIES OUTPUT_NAME "pgraster")
set_target_properties(input-pgraster PROPERTIES SUFFIX ".input")

mapnik_install(TARGET input-pgraster IS_PLUGIN)
