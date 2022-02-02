find_package(PostgreSQL REQUIRED)

add_plugin_target(input-postgis "postgis")
target_sources(input-postgis ${_plugin_visibility}
    postgis_datasource.cpp 
    postgis_featureset.cpp 
)
target_link_libraries(input-postgis ${_plugin_visibility}
    mapnik::mapnik  
    mapnik::datasource-base
    PostgreSQL::PostgreSQL
)
