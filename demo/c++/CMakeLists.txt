project(mapnik-demo)

add_executable(mapnik-demo rundemo.cpp)

target_link_libraries(mapnik-demo PRIVATE mapnik::core mapnik::agg mapnik::mapnik)

mapnik_install(TARGET mapnik-demo)
mapnik_copy_plugins(TARGET mapnik-demo DESTINATION plugins/input PLUGINS input-shape input-gdal)
mapnik_require_fonts(TARGET mapnik-demo DESTINATION fonts)
mapnik_copy_dependencies(TARGETS mapnik-demo PLUGINS input-shape)
