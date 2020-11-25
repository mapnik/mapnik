project(geometry_to_wkb)

add_executable(geometry_to_wkb main.cpp)

target_link_libraries(geometry_to_wkb PRIVATE 
    mapnik::headers 
    mapnik::mapnik
)

install(TARGETS geometry_to_wkb
    EXPORT MapnikTargets
    LIBRARY DESTINATION ${MAPNIK_LIB_DIR}
    ARCHIVE DESTINATION ${MAPNIK_ARCHIVE_DIR}
    RUNTIME DESTINATION ${MAPNIK_BIN_DIR}
)
