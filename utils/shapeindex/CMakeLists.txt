project(shapeindex)

find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS program_options)

add_executable(shapeindex 
    shapeindex.cpp
)

target_include_directories(shapeindex PRIVATE ../../plugins/input/shape)
target_link_libraries(shapeindex PRIVATE
    Boost::program_options
    mapnik::mapnik
)

set_target_properties(shapeindex PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
)

mapnik_install_utility(shapeindex)
