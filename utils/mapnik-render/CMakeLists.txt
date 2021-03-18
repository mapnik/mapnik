project(mapnik-render)

find_package(Boost 1.74 REQUIRED COMPONENTS program_options)

add_executable(mapnik-render mapnik-render.cpp)

target_link_libraries(mapnik-render PRIVATE 
    mapnik::core 
    mapnik::mapnik
    Boost::program_options
)
install(TARGETS mapnik-render
    EXPORT MapnikTargets
    LIBRARY DESTINATION ${MAPNIK_LIB_DIR}
    ARCHIVE DESTINATION ${MAPNIK_ARCHIVE_DIR}
    RUNTIME DESTINATION ${MAPNIK_BIN_DIR}
)
