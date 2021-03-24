project(mapnik-demo)

add_executable(mapnik-demo rundemo.cpp)

target_link_libraries(mapnik-demo PRIVATE mapnik::core mapnik::agg mapnik::mapnik)

install(TARGETS mapnik-demo 
    LIBRARY DESTINATION ${MAPNIK_LIB_DIR}
    ARCHIVE DESTINATION ${MAPNIK_ARCHIVE_DIR}
    RUNTIME DESTINATION ${MAPNIK_BIN_DIR}
)

if(COPY_LIBRARIES_FOR_EXECUTABLES AND WIN32)
    include(CopyDllsForDebug)
    add_custom_command(TARGET mapnik-demo POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:mapnik::mapnik>" ${CMAKE_CURRENT_BINARY_DIR})
    copy_dlls_for_debug(mapnik-demo \"\" \"${ADDITIONAL_LIBARIES_PATHS}\")
endif()
