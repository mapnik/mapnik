add_executable(mapnik-demo rundemo.cpp)
target_link_libraries(mapnik-demo PRIVATE mapnik::agg mapnik::mapnik)
