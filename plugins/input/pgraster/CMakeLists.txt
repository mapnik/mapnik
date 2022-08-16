mapnik_find_package(PostgreSQL REQUIRED)

add_plugin_target(input-pgraster "pgraster")
target_sources(input-pgraster ${_plugin_visibility}
    pgraster_datasource.cpp 
    pgraster_featureset.cpp 
    pgraster_wkb_reader.cpp
)
target_link_libraries(input-pgraster ${_plugin_visibility}
    mapnik::mapnik  
    mapnik::datasource-base
    PostgreSQL::PostgreSQL
)
