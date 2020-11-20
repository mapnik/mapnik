add_executable(ogrindex ogrindex.cpp)

target_link_libraries(ogrindex PRIVATE mapnik::mapnik)
install(TARGETS ogrindex
    EXPORT MapnikTargets
    RUNTIME DESTINATION bin
)