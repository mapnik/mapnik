project(geometry_to_wkb)

add_executable(geometry_to_wkb main.cpp)

target_link_libraries(geometry_to_wkb PRIVATE mapnik::mapnik)

mapnik_install(TARGET geometry_to_wkb)
