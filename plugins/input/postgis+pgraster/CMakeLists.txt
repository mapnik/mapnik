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
    /Users/artemp/projects/mapnik-consulting/mapnik-package-builder/_deps/lib/libgeos.a
    /Users/artemp/projects/mapnik-consulting/mapnik-package-builder/_deps/lib/libsqlite3.a
    PostgreSQL::PostgreSQL
)
