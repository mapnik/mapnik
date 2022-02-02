add_library(datasource-base INTERFACE)
add_library(mapnik::datasource-base ALIAS datasource-base)


target_include_directories(datasource-base INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(datasource-base INTERFACE mapnik::core)

mapnik_install(datasource-base)
install(DIRECTORY include/ DESTINATION "${MAPNIK_INCLUDE_DIR}")
