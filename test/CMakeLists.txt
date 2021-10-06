project(mapnik-test)

find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS program_options)
find_package(PostgreSQL REQUIRED)

include(FetchContent)
include(CopyDllsForDebug)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.13.7)
FetchContent_MakeAvailable(Catch2)

# Prepare "Catch" library for other executables
set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.19.0") 
    # for cmake 3.19+ https://cmake.org/cmake/help/git-stage/policy/CMP0110.html
    # might not be needed if catch updates its cmakes
    cmake_policy(SET CMP0110 OLD)
endif()


add_executable(mapnik-test-unit 
    unit/run.cpp
    unit/color/css_color.cpp
    unit/core/box2d_test.cpp
    unit/core/comparison_test.cpp
    unit/core/conversions_test.cpp
    unit/core/copy_move_test.cpp
    unit/core/exceptions_test.cpp
    unit/core/expressions_test.cpp
    unit/core/params_test.cpp
    unit/core/transform_expressions_test.cpp
    unit/core/value_test.cpp
    unit/datasource/csv.cpp
    unit/datasource/gdal.cpp
    unit/datasource/geobuf.cpp
    unit/datasource/geojson.cpp
    unit/datasource/memory.cpp
    unit/datasource/ogr.cpp
    unit/datasource/postgis.cpp
    unit/datasource/shapeindex.cpp
    unit/datasource/spatial_index.cpp
    unit/datasource/topojson.cpp
    unit/font/fontset_runtime_test.cpp
    unit/geometry/centroid.cpp
    unit/geometry/closest_point.cpp
    unit/geometry/geometry.cpp
    unit/geometry/geometry_envelope_test.cpp
    unit/geometry/geometry_hit_test.cpp
    unit/geometry/geometry_is_simple.cpp
    unit/geometry/geometry_is_valid.cpp
    unit/geometry/geometry_reprojection.cpp
    unit/geometry/geometry_strategy_test.cpp
    unit/geometry/geometry_test_helper.cpp
    unit/geometry/grid_vertex_converter.cpp
    unit/geometry/interior.cpp
    unit/geometry/is_clockwise.cpp
    unit/geometry/is_empty.cpp
    unit/geometry/polygon_vertex_processor.cpp
    unit/geometry/polylabel.cpp
    unit/geometry/remove_empty.cpp
    unit/imaging/image.cpp
    unit/imaging/image_apply_opacity.cpp
    unit/imaging/image_filter.cpp
    unit/imaging/image_io_test.cpp
    unit/imaging/image_is_solid.cpp
    unit/imaging/image_painted_test.cpp
    unit/imaging/image_premultiply.cpp
    unit/imaging/image_set_pixel.cpp
    unit/imaging/image_view.cpp
    unit/imaging/tiff_io.cpp
    unit/imaging/webp_io.cpp
    unit/map/background.cpp
    unit/numerics/enumeration.cpp
    unit/numerics/safe_cast.cpp
    unit/pixel/agg_blend_src_over_test.cpp
    unit/pixel/palette.cpp
    unit/projection/proj_transform.cpp
    unit/renderer/buffer_size_scale_factor.cpp
    unit/renderer/cairo_io.cpp
    unit/renderer/feature_style_processor.cpp
    unit/serialization/wkb_formats_test.cpp
    unit/serialization/wkb_test.cpp
    unit/serialization/xml_parser_trim.cpp
    unit/sql/sql_parse.cpp
    unit/svg/svg_parser_test.cpp
    unit/svg/svg_path_parser_test.cpp
    unit/svg/svg_renderer_test.cpp
    unit/symbolizer/marker_placement_vertex_last.cpp
    unit/symbolizer/markers_point_placement.cpp
    unit/symbolizer/symbolizer_test.cpp
    unit/text/script_runs.cpp
    unit/text/shaping.cpp
    unit/text/text_placements_list.cpp
    unit/text/text_placements_simple.cpp
    unit/util/char_array_buffer.cpp
    unit/vertex_adapter/clipping_test.cpp
    unit/vertex_adapter/extend_converter.cpp
    unit/vertex_adapter/line_offset_test.cpp
    unit/vertex_adapter/offset_converter.cpp
    unit/vertex_adapter/simplify_converters_test.cpp
    unit/vertex_adapter/transform_path_adapter.cpp
    unit/vertex_adapter/vertex_adapter.cpp
)
target_link_libraries(mapnik-test-unit PUBLIC
    Catch 
    mapnik::agg
    mapnik::mapnik
    mapnik::json
    mapnik::wkt
    PostgreSQL::PostgreSQL
)

add_executable(agg_rasterizer_integer_overflow_test standalone/agg_rasterizer_integer_overflow_test.cpp)
target_link_libraries(agg_rasterizer_integer_overflow_test PUBLIC
    Catch 
    mapnik::mapnik 
    mapnik::agg
    mapnik::json
)

add_executable(datasource_registration_test standalone/datasource_registration_test.cpp)
target_link_libraries(datasource_registration_test PUBLIC
    Catch 
    mapnik::mapnik 
    mapnik::agg
)

add_executable(font_registration_test standalone/font_registration_test.cpp)
target_link_libraries(font_registration_test PUBLIC
    Catch 
    mapnik::mapnik 
    mapnik::agg
    mapnik::json
)

#not workable since boost::filesystem native returns a wstring and the function taskes a std::string
add_executable(map_xml_test standalone/map_xml_test.cpp)
target_link_libraries(map_xml_test PUBLIC
    Catch 
    mapnik::mapnik 
    mapnik::agg
    mapnik::json
)

add_executable(mapnik-test-visual 
    visual/parse_map_sizes.cpp
    visual/report.cpp
    visual/runner.cpp
    visual/run.cpp
)
target_link_libraries(
    mapnik-test-visual PRIVATE
    Catch
    Boost::program_options
    Boost::filesystem
    mapnik::mapnik
    mapnik::agg
)

include(CTest)
include(${catch2_SOURCE_DIR}/contrib/Catch.cmake)
include(${catch2_SOURCE_DIR}/contrib/ParseAndAddCatchTests.cmake)

# use only mapnik-test-unit since it has the same build path
mapnik_copy_plugins(
    TARGET mapnik-test-unit 
    DESTINATION plugins/input 
    PLUGINS 
        input-csv input-gdal input-geobuf input-geojson input-ogr input-pgraster input-postgis input-raster input-shape input-sqlite input-topojson
)
mapnik_require_fonts(TARGET mapnik-test-unit DESTINATION fonts)

mapnik_copy_dependencies(
    TARGETS
        mapnik-test-unit agg_rasterizer_integer_overflow_test datasource_registration_test font_registration_test map_xml_test mapnik-test-visual
    PLUGINS 
        input-csv input-gdal input-geobuf input-geojson input-ogr input-pgraster input-postgis input-raster input-shape input-sqlite input-topojson
)


file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test)
file(COPY data-visual DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/test)

catch_discover_tests(mapnik-test-unit)
catch_discover_tests(agg_rasterizer_integer_overflow_test)
catch_discover_tests(datasource_registration_test)
catch_discover_tests(font_registration_test)
catch_discover_tests(map_xml_test)
