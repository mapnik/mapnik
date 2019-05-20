#CMake build, Pedro Vicente
cmake_minimum_required(VERSION 3.1)
project (mapnik)

set(CMAKE_CXX_STANDARD 11)
include_directories(include)
include_directories(deps)
include_directories(deps/agg/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
message("-- Source directory: " ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${BOOST_PREFIX})
include_directories(${ICU_INCLUDE})
include_directories(${FREE_TYPE_INCLUDE})
include_directories(${ZLIB_INCLUDE})
include_directories(${LIBICONV_INCLUDE})

if (MSVC)
add_definitions(-D_WINDOWS)
endif(MSVC)

#//////////////////////////
#mapnik core lib sources 
#${sources} contains a cascade definition of all the source files
#//////////////////////////

set(sources ${sources})

#//////////////////////////
#LIBXML2 
#//////////////////////////

include_directories(${LIBXML2_INCLUDE})
add_definitions(-DHAVE_LIBXML2)
set(sources ${sources} src/libxml2_loader.cpp)


#//////////////////////////
#mapnik core lib sources 
#//////////////////////////

set(sources ${sources} src/fs.cpp)
set(sources ${sources} src/debug_symbolizer.cpp)
set(sources ${sources} src/request.cpp)
set(sources ${sources} src/well_known_srs.cpp)
set(sources ${sources} src/params.cpp)
set(sources ${sources} src/image_filter_types.cpp)
set(sources ${sources} src/miniz_png.cpp)
set(sources ${sources} src/color.cpp)
set(sources ${sources} src/css_color_grammar.cpp)
set(sources ${sources} src/conversions.cpp)
set(sources ${sources} src/image_compositing.cpp)
set(sources ${sources} src/image_filter_grammar.cpp)
set(sources ${sources} src/image_scaling.cpp)
set(sources ${sources} src/box2d.cpp)
set(sources ${sources} src/building_symbolizer.cpp)
set(sources ${sources} src/datasource_cache.cpp)
set(sources ${sources} src/datasource_cache_static.cpp)
set(sources ${sources} src/debug.cpp)
set(sources ${sources} src/expression_node.cpp)
set(sources ${sources} src/expression_grammar.cpp)
set(sources ${sources} src/expression_string.cpp)
set(sources ${sources} src/expression.cpp)
set(sources ${sources} src/transform_expression_grammar.cpp)
set(sources ${sources} src/transform_expression.cpp)
set(sources ${sources} src/feature_kv_iterator.cpp)
set(sources ${sources} src/feature_style_processor.cpp)
set(sources ${sources} src/feature_type_style.cpp)
set(sources ${sources} src/font_engine_freetype.cpp)
set(sources ${sources} src/font_set.cpp)
set(sources ${sources} src/gamma_method.cpp)
set(sources ${sources} src/gradient.cpp)
set(sources ${sources} src/graphics.cpp)
set(sources ${sources} src/image_reader.cpp)
set(sources ${sources} src/image_util.cpp)
set(sources ${sources} src/layer.cpp)
set(sources ${sources} src/line_symbolizer.cpp)
set(sources ${sources} src/line_pattern_symbolizer.cpp)
set(sources ${sources} src/map.cpp)
set(sources ${sources} src/load_map.cpp)
set(sources ${sources} src/memory.cpp)
set(sources ${sources} src/parse_path.cpp)
set(sources ${sources} src/parse_transform.cpp)
set(sources ${sources} src/palette.cpp)
set(sources ${sources} src/path_expression_grammar.cpp)
set(sources ${sources} src/placement_finder.cpp)
set(sources ${sources} src/plugin.cpp)
set(sources ${sources} src/point_symbolizer.cpp)
set(sources ${sources} src/polygon_pattern_symbolizer.cpp)
set(sources ${sources} src/polygon_symbolizer.cpp)
set(sources ${sources} src/rule.cpp)
set(sources ${sources} src/save_map.cpp)
set(sources ${sources} src/shield_symbolizer.cpp)
set(sources ${sources} src/text_symbolizer.cpp)
set(sources ${sources} src/wkb.cpp)
set(sources ${sources} src/projection.cpp)
set(sources ${sources} src/proj_transform.cpp)
set(sources ${sources} src/distance.cpp)
set(sources ${sources} src/scale_denominator.cpp)
set(sources ${sources} src/simplify.cpp)
set(sources ${sources} src/memory_datasource.cpp)
set(sources ${sources} src/stroke.cpp)
set(sources ${sources} src/symbolizer.cpp)
set(sources ${sources} src/symbolizer_helpers.cpp)
set(sources ${sources} src/unicode.cpp)
set(sources ${sources} src/markers_symbolizer.cpp)
set(sources ${sources} src/raster_colorizer.cpp)
set(sources ${sources} src/raster_symbolizer.cpp)
set(sources ${sources} src/wkt/wkt_factory.cpp)
set(sources ${sources} src/wkt/wkt_generator.cpp)
set(sources ${sources} src/mapped_memory_cache.cpp)
set(sources ${sources} src/marker_cache.cpp)
set(sources ${sources} src/svg/svg_parser.cpp)
set(sources ${sources} src/svg/svg_path_parser.cpp)
set(sources ${sources} src/svg/svg_points_parser.cpp)
set(sources ${sources} src/svg/svg_transform_parser.cpp)
set(sources ${sources} src/warp.cpp)
set(sources ${sources} src/json/geometry_grammar.cpp)
set(sources ${sources} src/json/geometry_parser.cpp)
set(sources ${sources} src/json/feature_grammar.cpp)
set(sources ${sources} src/json/feature_parser.cpp)
set(sources ${sources} src/json/feature_collection_parser.cpp)
set(sources ${sources} src/json/geojson_generator.cpp)
set(sources ${sources} src/processed_text.cpp)
set(sources ${sources} src/formatting/base.cpp)
set(sources ${sources} src/formatting/expression.cpp)
set(sources ${sources} src/formatting/list.cpp)
set(sources ${sources} src/formatting/text.cpp)
set(sources ${sources} src/formatting/format.cpp)
set(sources ${sources} src/formatting/registry.cpp)
set(sources ${sources} src/text_placements/registry.cpp)
set(sources ${sources} src/text_placements/base.cpp)
set(sources ${sources} src/text_placements/dummy.cpp)
set(sources ${sources} src/text_placements/list.cpp)
set(sources ${sources} src/text_placements/simple.cpp)
set(sources ${sources} src/text_properties.cpp)
set(sources ${sources} src/xml_tree.cpp)
set(sources ${sources} src/config_error.cpp)
set(sources ${sources} src/color_factory.cpp)
#for windows
set(sources ${sources} src/utils.cpp)


#//////////////////////////
#mapnik agg renderer sources 
#//////////////////////////

set(sources ${sources} src/agg/agg_renderer.cpp)
set(sources ${sources} src/agg/process_building_symbolizer.cpp)
set(sources ${sources} src/agg/process_line_symbolizer.cpp)
set(sources ${sources} src/agg/process_line_pattern_symbolizer.cpp)
set(sources ${sources} src/agg/process_text_symbolizer.cpp)
set(sources ${sources} src/agg/process_point_symbolizer.cpp)
set(sources ${sources} src/agg/process_polygon_symbolizer.cpp)
set(sources ${sources} src/agg/process_polygon_pattern_symbolizer.cpp)
set(sources ${sources} src/agg/process_raster_symbolizer.cpp)
set(sources ${sources} src/agg/process_shield_symbolizer.cpp)
set(sources ${sources} src/agg/process_markers_symbolizer.cpp)
set(sources ${sources} src/agg/process_debug_symbolizer.cpp)

#//////////////////////////
#agg library in dep 
#//////////////////////////

set(sources ${sources} deps/agg/src/agg_arc.cpp)
set(sources ${sources} deps/agg/src/agg_image_filters.cpp)
set(sources ${sources} deps/agg/src/agg_trans_double_path.cpp)
set(sources ${sources} deps/agg/src/agg_vcgen_smooth_poly1.cpp)
set(sources ${sources} deps/agg/src/agg_arrowhead.cpp)
set(sources ${sources} deps/agg/src/agg_line_aa_basics.cpp)
set(sources ${sources} deps/agg/src/agg_trans_single_path.cpp)
set(sources ${sources} deps/agg/src/agg_vcgen_stroke.cpp)
set(sources ${sources} deps/agg/src/agg_bezier_arc.cpp)
set(sources ${sources} deps/agg/src/agg_line_profile_aa.cpp)
set(sources ${sources} deps/agg/src/agg_trans_warp_magnifier.cpp)
set(sources ${sources} deps/agg/src/agg_vpgen_clip_polygon.cpp)
set(sources ${sources} deps/agg/src/agg_bspline.cpp)
set(sources ${sources} deps/agg/src/agg_pixfmt_rgba.cpp)
set(sources ${sources} deps/agg/src/agg_vcgen_bspline.cpp)
set(sources ${sources} deps/agg/src/agg_vpgen_clip_polyline.cpp)
set(sources ${sources} deps/agg/src/agg_curves.cpp)
set(sources ${sources} deps/agg/src/agg_rounded_rect.cpp)
set(sources ${sources} deps/agg/src/agg_vcgen_contour.cpp)
set(sources ${sources} deps/agg/src/agg_vpgen_segmentator.cpp)
set(sources ${sources} deps/agg/src/agg_embedded_raster_fonts.cpp)
set(sources ${sources} deps/agg/src/agg_sqrt_tables.cpp)
set(sources ${sources} deps/agg/src/agg_vcgen_dash.cpp)
set(sources ${sources} deps/agg/src/agg_gsv_text.cpp)
set(sources ${sources} deps/agg/src/agg_trans_affine.cpp)
set(sources ${sources} deps/agg/src/agg_vcgen_markers_term.cpp)

#//////////////////////////
#mapnik library 
#//////////////////////////

add_library(mapnik STATIC ${sources})
set(mapnik_lib mapnik)

#//////////////////////////
#link with libraries
#lib_dep contains a cascade definition of all the libraries needed to link
#//////////////////////////

set(lib_dep ${lib_dep})
set(lib_dep ${lib_dep} ${libboost_regex})
set(lib_dep ${lib_dep} ${libboost_filesystem})
set(lib_dep ${lib_dep} ${libboost_system})
set(lib_dep ${lib_dep} ${LIB_ICU})
set(lib_dep ${lib_dep} ${LIB_FREETYPE})
set(lib_dep ${lib_dep} ${LIB_LIBXML2})

#//////////////////////////
#OSM plugin 
#plugin depends on libmapnik and its dependencies
#//////////////////////////

set(osm_sources ${osm_sources})
set(osm_sources ${osm_sources} plugins/input/osm/osm.cpp)
set(osm_sources ${osm_sources} plugins/input/osm/osm_datasource.cpp)
set(osm_sources ${osm_sources} plugins/input/osm/osm_featureset.cpp)
set(osm_sources ${osm_sources} plugins/input/osm/osmparser.cpp)
set(osm_sources ${osm_sources} plugins/input/osm/dataset_deliverer.cpp)
add_library(osm SHARED ${osm_sources})
target_link_libraries(osm ${mapnik_lib} ${lib_dep})
message("-- Building OSM plugin...")

#//////////////////////////
#demo program
#//////////////////////////

add_executable(demo demo/c++/rundemo.cpp)
target_link_libraries (demo ${mapnik_lib} ${lib_dep})

#//////////////////////////
#example program
#//////////////////////////

add_executable(example_01 example/example_01.cpp)
target_link_libraries (example_01 ${mapnik_lib} ${lib_dep})
