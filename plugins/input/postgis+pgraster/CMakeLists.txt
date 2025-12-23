mapnik_find_package(PostgreSQL REQUIRED)

add_plugin_target(input-postgis+pgraster "postgis+pgraster")
target_sources(input-postgis+pgraster ${_plugin_visibility}
    postgis_datasource.cpp
    postgis_featureset.cpp
    pgraster_datasource.cpp
    pgraster_featureset.cpp
    pgraster_wkb_reader.cpp
)
target_link_libraries(input-postgis+pgraster ${_plugin_visibility}
    mapnik::mapnik
    PostgreSQL::PostgreSQL
)
