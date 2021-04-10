project(svg2png)

find_package(Boost 1.74 REQUIRED COMPONENTS program_options)

add_executable(svg2png 
    svg2png.cpp
)

target_link_libraries(svg2png PRIVATE 
    Boost::program_options      
    mapnik::core 
    mapnik::mapnik
    mapnik::agg
)

mapnik_install(TARGET svg2png)
