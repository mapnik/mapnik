find_package(SQLite3 REQUIRED)

add_plugin_target(input-sqlite "sqlite")
target_sources(input-sqlite ${_plugin_visibility}
    sqlite_datasource.cpp
    sqlite_featureset.cpp
)
target_link_libraries(input-sqlite ${_plugin_visibility}
    mapnik::mapnik
    SQLite::SQLite3
)
