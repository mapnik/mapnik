add_executable(ogrindex ogrindex.cpp)

target_link_libraries(ogrindex PRIVATE mapnik::mapnik)

set_target_properties(ogrindex PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<BOOL:${BUILD_SHARED_CRT}>:DLL>"
)

mapnik_install_utility(ogrindex)
