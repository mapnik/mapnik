project(shapeindex)

find_package(Boost 1.74 REQUIRED COMPONENTS program_options)

add_executable(shapeindex 
    shapeindex.cpp
)

target_include_directories(shapeindex PRIVATE ../../plugins/input/shape)
target_link_libraries(shapeindex PRIVATE
    Boost::program_options
    mapnik::core 
    mapnik::mapnik
)
install(TARGETS shapeindex
    EXPORT MapnikTargets
    LIBRARY DESTINATION ${MAPNIK_LIB_DIR}
    ARCHIVE DESTINATION ${MAPNIK_ARCHIVE_DIR}
    RUNTIME DESTINATION ${MAPNIK_BIN_DIR}
)
