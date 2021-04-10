project(input-postgis)

find_package(PostgreSQL REQUIRED)

add_library(input-postgis SHARED 
    postgis_datasource.cpp 
    postgis_featureset.cpp 
)
add_library(mapnik::plugin::input::postgis ALIAS input-postgis)

target_link_libraries(input-postgis PRIVATE 
    mapnik::core 
    mapnik::mapnik  
    PostgreSQL::PostgreSQL
)

set_target_properties(input-postgis PROPERTIES PREFIX "")
set_target_properties(input-postgis PROPERTIES OUTPUT_NAME "postgis")
set_target_properties(input-postgis PROPERTIES SUFFIX ".input")

mapnik_install(TARGET input-postgis IS_PLUGIN)
