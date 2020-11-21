cmake_minimum_required(VERSION 3.15.0)
project(input-pgraster)

find_package(PostgreSQL REQUIRED)

add_library(input-pgraster SHARED 
    pgraster_datasource.cpp 
    pgraster_featureset.cpp 
    pgraster_wkb_reader.cpp
)
add_library(mapnik::plugin::input::pgraster ALIAS input-pgraster)


target_link_libraries(input-pgraster PRIVATE 
    mapnik::headers 
    mapnik::mapnik  
    PostgreSQL::PostgreSQL
)
set_target_properties(input-pgraster PROPERTIES PREFIX "")
set_target_properties(input-pgraster PROPERTIES OUTPUT_NAME "pgraster")
set_target_properties(input-pgraster PROPERTIES SUFFIX ".input")

install(TARGETS input-pgraster 
    EXPORT MapnikTargets
    LIBRARY DESTINATION lib/plugins/input
    ARCHIVE DESTINATION lib/plugins/input
    RUNTIME DESTINATION bin/plugins/input
    INCLUDES DESTINATION .
    PUBLIC_HEADER DESTINATION .
)
