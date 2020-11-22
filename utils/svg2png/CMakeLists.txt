project(svg2png)

find_package(Boost 1.74 REQUIRED COMPONENTS program_options)

add_executable(svg2png 
    svg2png.cpp
)

target_link_libraries(svg2png PRIVATE 
    Boost::program_options      
    mapnik::headers 
    mapnik::mapnik
    mapnik::agg
)
install(TARGETS svg2png
    EXPORT MapnikTargets
    RUNTIME DESTINATION bin
)