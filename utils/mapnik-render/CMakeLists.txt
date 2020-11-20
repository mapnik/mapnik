add_executable(mapnik-render mapnik-render.cpp)

target_link_libraries(mapnik-render PRIVATE mapnik::mapnik)
install(TARGETS mapnik-render
    EXPORT MapnikTargets
    RUNTIME DESTINATION bin
)