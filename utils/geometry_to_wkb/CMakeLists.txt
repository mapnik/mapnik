add_executable(geometry_to_wkb main.cpp)

target_link_libraries(geometry_to_wkb PRIVATE mapnik::headers mapnik::mapnik)

install(TARGETS geometry_to_wkb
    EXPORT MapnikTargets
    RUNTIME DESTINATION bin
)