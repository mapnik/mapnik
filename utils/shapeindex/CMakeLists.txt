add_executable(shapeindex 
    shapeindex.cpp
)

target_include_directories(shapeindex PRIVATE ../../plugins/input/shape)
target_link_libraries(shapeindex PRIVATE
    Boost::program_options
    mapnik::headers 
    mapnik::mapnik
)
install(TARGETS shapeindex
    EXPORT MapnikTargets
    RUNTIME DESTINATION bin
)