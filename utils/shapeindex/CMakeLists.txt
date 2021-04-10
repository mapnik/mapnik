project(shapeindex)

find_package(Boost 1.74 REQUIRED COMPONENTS program_options)

add_executable(shapeindex 
    shapeindex.cpp
)

target_include_directories(shapeindex PRIVATE ../../plugins/input/shape)
target_link_libraries(shapeindex PRIVATE
    Boost::program_options
    mapnik::core 
    mapnik::mapnik
)

mapnik_install(TARGET shapeindex)
