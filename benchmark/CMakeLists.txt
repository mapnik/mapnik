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
function(mapnik_create_benchmark)
    get_filename_component(BENCHNAME ${ARGV0} NAME_WE)
    set(TARGET_NAME "mapnik-benchmark-${BENCHNAME}")
    add_executable(${TARGET_NAME} ${ARGV0})
    target_include_directories(${TARGET_NAME} PRIVATE include)
    target_link_libraries(${TARGET_NAME} PRIVATE mapnik::core mapnik::agg mapnik::mapnik)
    
    # it might be possible that some buildsystems generate different directories. So invoke the scripts for all benchmarks
    mapnik_copy_plugins(TARGET ${TARGET_NAME}
        DESTINATION plugins/input 
        PLUGINS 
            input-csv input-gdal input-geobuf input-geojson input-ogr input-pgraster input-postgis input-raster input-shape input-sqlite input-topojson
    )
    mapnik_require_fonts(TARGET ${TARGET_NAME} mapnik-test-unit DESTINATION fonts)

    mapnik_copy_dependencies(TARGETS ${TARGET_NAME}
        PLUGINS 
            input-csv input-gdal input-geobuf input-geojson input-ogr input-pgraster input-postgis input-raster input-shape input-sqlite input-topojson
    )
endfunction()

foreach(benchmark ${BENCHMARK_SRCS})
    mapnik_create_benchmark(${benchmark})
endforeach()
