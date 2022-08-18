add_executable(ogrindex ogrindex.cpp)

target_link_libraries(ogrindex PRIVATE mapnik::mapnik)

mapnik_install_utility(ogrindex)
