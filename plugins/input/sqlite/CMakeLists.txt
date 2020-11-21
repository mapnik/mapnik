cmake_minimum_required(VERSION 3.15.0)
project(input-sqlite)

find_package(SQLite3 REQUIRED)

add_library(input-sqlite SHARED
    sqlite_datasource.cpp
    sqlite_featureset.cpp
)
add_library(mapnik::plugin::input::sqlite ALIAS input-sqlite)

target_link_libraries(input-sqlite PRIVATE 
    mapnik::headers 
    mapnik::mapnik 
    SQLite::SQLite3
)

set_target_properties(input-sqlite PROPERTIES PREFIX "")
set_target_properties(input-sqlite PROPERTIES OUTPUT_NAME "sqlite")
set_target_properties(input-sqlite PROPERTIES SUFFIX ".input")

install(TARGETS input-sqlite 
    EXPORT MapnikTargets
    LIBRARY DESTINATION lib/plugins/input
    ARCHIVE DESTINATION lib/plugins/input
    RUNTIME DESTINATION bin/plugins/input
    INCLUDES DESTINATION .
    PUBLIC_HEADER DESTINATION .
)
