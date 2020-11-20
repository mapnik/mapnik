cmake_minimum_required(VERSION 3.15.0)
project(agg)

add_library(agg INTERFACE)
add_library(mapnik::agg ALIAS agg)

target_include_directories(agg INTERFACE  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(agg INTERFACE mapnik::headers)

install(TARGETS agg
    EXPORT MapnikTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include
)
# todo install headers
