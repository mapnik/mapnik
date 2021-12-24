add_executable(geometry_to_wkb main.cpp)

target_link_libraries(geometry_to_wkb PRIVATE mapnik::mapnik)

set_target_properties(geometry_to_wkb PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
)

mapnik_install_utility(geometry_to_wkb)
