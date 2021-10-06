project(input-postgis)

find_package(PostgreSQL REQUIRED)

add_library(input-postgis MODULE 
    postgis_datasource.cpp 
    postgis_featureset.cpp 
)
target_link_libraries(input-postgis PRIVATE
    mapnik::mapnik  
    PostgreSQL::PostgreSQL
)
set_target_properties(input-postgis PROPERTIES 
    OUTPUT_NAME "postgis"
    PREFIX "${_plugin_prefix}"
    SUFFIX "${_plugin_suffix}"
)

mapnik_install_plugin(input-postgis)
