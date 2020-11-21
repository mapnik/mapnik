cmake_minimum_required(VERSION 3.15.0)
project(input-postgis)

find_package(PostgreSQL REQUIRED)

add_library(input-postgis SHARED 
    postgis_datasource.cpp 
    postgis_featureset.cpp 
)
add_library(mapnik::plugin::input::postgis ALIAS input-postgis)

target_link_libraries(input-postgis PRIVATE 
    mapnik::headers 
    mapnik::mapnik  
    PostgreSQL::PostgreSQL
)

set_target_properties(input-postgis PROPERTIES PREFIX "")
set_target_properties(input-postgis PROPERTIES OUTPUT_NAME "postgis")
set_target_properties(input-postgis PROPERTIES SUFFIX ".input")

install(TARGETS input-postgis 
    LIBRARY DESTINATION lib/plugins/input
    ARCHIVE DESTINATION lib/plugins/input
    RUNTIME DESTINATION bin/plugins/input
)
