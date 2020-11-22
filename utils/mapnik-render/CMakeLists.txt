project(mapnik-render)

find_package(Boost 1.74 REQUIRED COMPONENTS program_options)

add_executable(mapnik-render mapnik-render.cpp)

target_link_libraries(mapnik-render PRIVATE 
    mapnik::headers 
    mapnik::mapnik
    Boost::program_options
)
install(TARGETS mapnik-render
    EXPORT MapnikTargets
    RUNTIME DESTINATION bin
)