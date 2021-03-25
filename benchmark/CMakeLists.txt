project(mapnik-benchmark)

set(BENCHMARK_SRCS 
    src/normalize_angle.cpp
    src/test_array_allocation.cpp
    src/test_expression_parse.cpp
    src/test_face_ptr_creation.cpp
    src/test_font_registration.cpp
    src/test_getline.cpp
    src/test_marker_cache.cpp
    src/test_noop_rendering.cpp
    src/test_numeric_cast_vs_static_cast.cpp
    src/test_offset_converter.cpp
    src/test_png_encoding1.cpp
    src/test_png_encoding2.cpp
    src/test_polygon_clipping_rendering.cpp
    src/test_polygon_clipping.cpp
    src/test_proj_transform1.cpp
    src/test_quad_tree.cpp
    src/test_rendering_shared_map.cpp
    src/test_rendering.cpp
    src/test_to_bool.cpp
    src/test_to_double.cpp
    src/test_to_int.cpp
    src/test_to_string1.cpp
    src/test_to_string2.cpp
    src/test_utf_encoding.cpp
)
macro(mapnik_create_benchmark)
    get_filename_component(BENCHNAME ${ARGV0} NAME_WE)
    add_executable(mapnik-benchmark-${BENCHNAME} ${ARGV0})
    target_include_directories(mapnik-benchmark-${BENCHNAME} PRIVATE include)
    target_link_libraries(mapnik-benchmark-${BENCHNAME} PRIVATE mapnik::core mapnik::agg mapnik::mapnik)

    if(COPY_LIBRARIES_FOR_EXECUTABLES AND WIN32)
        include(CopyDllsForDebug)
        add_custom_command(TARGET mapnik-benchmark-${BENCHNAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:mapnik::mapnik>" ${CMAKE_CURRENT_BINARY_DIR})
        copy_dlls_for_debug(mapnik-benchmark-${BENCHNAME} \"\" \"${ADDITIONAL_LIBARIES_PATHS}\")
    endif()
endmacro()

foreach(benchmark ${BENCHMARK_SRCS})
    mapnik_create_benchmark(${benchmark})
endforeach()
