project(ogrindex)

add_executable(ogrindex ogrindex.cpp)

target_link_libraries(ogrindex PRIVATE mapnik::core mapnik::mapnik)

mapnik_install(TARGET ogrindex)
