find_package(SQLite3 REQUIRED)

add_library(input-sqlite ${_plugin_linkage})
target_sources(input-sqlite ${_plugin_visibility}
    sqlite_datasource.cpp
    sqlite_featureset.cpp
)
target_link_libraries(input-sqlite ${_plugin_visibility}
    mapnik::mapnik 
    mapnik::datasource-base
    SQLite::SQLite3
)
set_target_properties(input-sqlite PROPERTIES 
    OUTPUT_NAME "sqlite"
    PREFIX "${_plugin_prefix}"
    SUFFIX "${_plugin_suffix}"
    LIBRARY_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/plugins/input"
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/lib"
)
mapnik_install_plugin(input-sqlite)
