add_executable(mapnik-demo rundemo.cpp)
set_target_properties(mapnik-demo PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/lib"
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<BOOL:${BUILD_SHARED_CRT}>:DLL>"
)

target_link_libraries(mapnik-demo PRIVATE mapnik::agg mapnik::mapnik)
