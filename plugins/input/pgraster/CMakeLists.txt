project(input-pgraster)

find_package(PostgreSQL REQUIRED)

add_library(input-pgraster MODULE 
    pgraster_datasource.cpp 
    pgraster_featureset.cpp 
    pgraster_wkb_reader.cpp
)
target_link_libraries(input-pgraster PRIVATE
    mapnik::mapnik  
    PostgreSQL::PostgreSQL
)
set_target_properties(input-pgraster PROPERTIES 
    OUTPUT_NAME "pgraster"
    PREFIX "${_plugin_prefix}"
    SUFFIX "${_plugin_suffix}"
    LIBRARY_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/plugins/input"
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/lib"
)
mapnik_install_plugin(input-pgraster)
