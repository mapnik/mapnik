project(input-sqlite)

find_package(SQLite3 REQUIRED)

add_library(input-sqlite MODULE
    sqlite_datasource.cpp
    sqlite_featureset.cpp
)
add_library(mapnik::plugin::input::sqlite ALIAS input-sqlite)
target_link_libraries(input-sqlite PRIVATE
    mapnik::mapnik 
    SQLite::SQLite3
)
set_target_properties(input-sqlite PROPERTIES 
    OUTPUT_NAME "sqlite"
    PREFIX "${_plugin_prefix}"
    SUFFIX "${_plugin_suffix}"
)

mapnik_install_plugin(input-sqlite)
