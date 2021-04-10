project(ogrindex)

add_executable(ogrindex ogrindex.cpp)


target_link_libraries(ogrindex PRIVATE mapnik::core mapnik::mapnik)

install(TARGETS ogrindex
    EXPORT MapnikTargets
    LIBRARY DESTINATION ${MAPNIK_LIB_DIR}
    ARCHIVE DESTINATION ${MAPNIK_ARCHIVE_DIR}
    RUNTIME DESTINATION ${MAPNIK_BIN_DIR}
)

mapnik_install(TARGET mapnik-ogrindex)
