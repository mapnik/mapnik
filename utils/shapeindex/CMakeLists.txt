add_executable(shapeindex 
    shapeindex.cpp
)

target_link_libraries(shapeindex PRIVATE mapnik::mapnik)
install(TARGETS shapeindex
    EXPORT MapnikTargets
    RUNTIME DESTINATION bin
)