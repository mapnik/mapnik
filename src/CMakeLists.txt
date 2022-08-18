add_subdirectory(json)
add_subdirectory(wkt)

if(BUILD_SHARED_LIBS)
    # private as this should only be visibile when building, to dllexport/dllimport correctly
    target_compile_definitions(mapnik PRIVATE MAPNIK_EXPORTS)
endif()

target_link_libraries(mapnik PUBLIC mapnik::core mapnik::datasource-base)
target_link_libraries(mapnik PRIVATE
    mapnik::agg
    # expr: if(BUILD_SHARED_PLUGINS == OFF && <target> is build) then add link target
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-csv>>:input-csv>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-gdal>>:input-gdal>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-geobuf>>:input-geobuf>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-geojson>>:input-geojson>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-ogr>>:input-ogr>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-pgraster>>:input-pgraster>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-postgis>>:input-postgis>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-raster>>:input-raster>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-shape>>:input-shape>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-sqlite>>:input-sqlite>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-topojson>>:input-topojson>
)
target_compile_definitions(mapnik PRIVATE
    # expr: if(BUILD_SHARED_PLUGINS == OFF && <target> is build) then add build definition
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-csv>>:MAPNIK_STATIC_PLUGIN_CSV>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-gdal>>:MAPNIK_STATIC_PLUGIN_GDAL>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-geobuf>>:MAPNIK_STATIC_PLUGIN_GEOBUF>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-geojson>>:MAPNIK_STATIC_PLUGIN_GEOJSON>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-ogr>>:MAPNIK_STATIC_PLUGIN_OGR>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-pgraster>>:MAPNIK_STATIC_PLUGIN_PGRASTER>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-postgis>>:MAPNIK_STATIC_PLUGIN_POSTGIS>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-raster>>:MAPNIK_STATIC_PLUGIN_RASTER>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-shape>>:MAPNIK_STATIC_PLUGIN_SHAPE>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-sqlite>>:MAPNIK_STATIC_PLUGIN_SQLITE>
    $<$<AND:$<NOT:$<BOOL:${BUILD_SHARED_PLUGINS}>>,$<TARGET_EXISTS:input-topojson>>:MAPNIK_STATIC_PLUGIN_TOPOJSON>

)

if(NOT BUILD_SHARED_PLUGINS)
    target_include_directories(mapnik PRIVATE "${PROJECT_SOURCE_DIR}/plugins")
endif()

set_target_properties(mapnik PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME "mapnik"
    PREFIX "lib"
    IMPORT_PREFIX "lib" # for the archive on dll platforms
    VERSION ${MAPNIK_VERSION}
    # see https://github.com/mapnik/mapnik/pull/4248#issuecomment-925596509 => ABI compability only with the full version
    SOVERSION ${MAPNIK_VERSION}
)

if(MSVC)
    # disable some msvc warnings and enable bigobj
    # 4068 will warn about unknown definitions. Would not be needed if https://github.com/mapbox/geometry.hpp/pull/69 is merged.
    # 4661 warns about enum definitions. should be investigated at some point
    # 4910 warns about extern and dllexport at the same time. should be investigated at some point
    target_compile_options(mapnik PUBLIC "/bigobj" "/wd4068" "/wd4661" "/wd4910")
endif()

target_sources(mapnik PRIVATE
    cairo_io.cpp
    color_factory.cpp
    color.cpp
    config_error.cpp
    conversions_numeric.cpp
    conversions_string.cpp
    dasharray_parser.cpp
    datasource_cache_static.cpp
    datasource_cache.cpp
    debug.cpp
    expression_grammar_x3.cpp
    expression_node.cpp
    expression_string.cpp
    expression.cpp
    feature_kv_iterator.cpp
    feature_style_processor.cpp
    feature_type_style.cpp
    font_engine_freetype.cpp
    font_set.cpp
    fs.cpp
    function_call.cpp
    generate_image_filters.cpp
    gradient.cpp
    image_any.cpp
    image_compositing.cpp
    image_copy.cpp
    image_filter_grammar_x3.cpp
    image_options.cpp
    image_reader.cpp
    image_scaling.cpp
    image_util_jpeg.cpp
    image_util_png.cpp
    image_util_tiff.cpp
    image_util_webp.cpp
    image_util.cpp
    image_view_any.cpp
    image_view.cpp
    image.cpp
    layer.cpp
    load_map.cpp
    map.cpp
    mapnik.cpp
    mapped_memory_cache.cpp
    marker_cache.cpp
    marker_helpers.cpp
    memory_datasource.cpp
    palette.cpp
    params.cpp
    parse_image_filters.cpp
    parse_path.cpp
    parse_transform.cpp
    path_expression_grammar_x3.cpp
    plugin.cpp
    proj_transform_cache.cpp
    proj_transform.cpp
    projection.cpp
    raster_colorizer.cpp
    renderer_common.cpp
    request.cpp
    rule.cpp
    save_map.cpp
    scale_denominator.cpp
    simplify.cpp
    symbolizer_enumerations.cpp
    symbolizer_keys.cpp
    symbolizer.cpp
    transform_expression_grammar_x3.cpp
    transform_expression.cpp
    twkb.cpp
    unicode.cpp
    value.cpp
    vertex_adapters.cpp
    vertex_cache.cpp
    warp.cpp
    well_known_srs.cpp
    wkb.cpp
    xml_tree.cpp
)
target_sources(mapnik PRIVATE
    ../deps/agg/src/agg_arc.cpp
    ../deps/agg/src/agg_arrowhead.cpp
    ../deps/agg/src/agg_bezier_arc.cpp
    ../deps/agg/src/agg_bspline.cpp
    ../deps/agg/src/agg_curves.cpp
    ../deps/agg/src/agg_embedded_raster_fonts.cpp
    ../deps/agg/src/agg_gsv_text.cpp
    ../deps/agg/src/agg_image_filters.cpp
    ../deps/agg/src/agg_line_aa_basics.cpp
    ../deps/agg/src/agg_line_profile_aa.cpp
    ../deps/agg/src/agg_pixfmt_rgba.cpp
    ../deps/agg/src/agg_rounded_rect.cpp
    ../deps/agg/src/agg_sqrt_tables.cpp
    ../deps/agg/src/agg_trans_affine.cpp
    ../deps/agg/src/agg_trans_double_path.cpp
    ../deps/agg/src/agg_trans_single_path.cpp
    ../deps/agg/src/agg_trans_warp_magnifier.cpp
    ../deps/agg/src/agg_vcgen_bspline.cpp
    ../deps/agg/src/agg_vcgen_contour.cpp
    ../deps/agg/src/agg_vcgen_dash.cpp
    ../deps/agg/src/agg_vcgen_markers_term.cpp
    ../deps/agg/src/agg_vcgen_stroke.cpp
    ../deps/agg/src/agg_vpgen_clip_polygon.cpp
    ../deps/agg/src/agg_vpgen_clip_polyline.cpp
    ../deps/agg/src/agg_vpgen_segmentator.cpp
)
target_sources(mapnik PRIVATE
    agg/agg_renderer.cpp
    agg/process_building_symbolizer.cpp
    agg/process_debug_symbolizer.cpp
    agg/process_dot_symbolizer.cpp
    agg/process_group_symbolizer.cpp
    agg/process_line_pattern_symbolizer.cpp
    agg/process_line_symbolizer.cpp
    agg/process_markers_symbolizer.cpp
    agg/process_point_symbolizer.cpp
    agg/process_polygon_pattern_symbolizer.cpp
    agg/process_polygon_symbolizer.cpp
    agg/process_raster_symbolizer.cpp
    agg/process_shield_symbolizer.cpp
    agg/process_text_symbolizer.cpp
)

target_sources(mapnik PRIVATE
    css/css_color_grammar_x3.cpp
    css/css_grammar_x3.cpp
)

target_sources(mapnik PRIVATE
    geometry/box2d.cpp
    geometry/closest_point.cpp
    geometry/envelope.cpp
    geometry/interior.cpp
    geometry/polylabel.cpp
    geometry/reprojection.cpp
)

target_sources(mapnik PRIVATE
    renderer_common/pattern_alignment.cpp
    renderer_common/render_group_symbolizer.cpp
    renderer_common/render_markers_symbolizer.cpp
    renderer_common/render_pattern.cpp
    renderer_common/render_thunk_extractor.cpp
)

target_sources(mapnik PRIVATE
    svg/svg_parser.cpp
    svg/svg_path_grammar_x3.cpp
    svg/svg_path_parser.cpp
    svg/svg_points_parser.cpp
    svg/svg_transform_parser.cpp
)

target_sources(mapnik PRIVATE
    group/group_layout_manager.cpp
    group/group_rule.cpp
    group/group_symbolizer_helper.cpp
)

target_sources(mapnik PRIVATE
    text/color_font_renderer.cpp
    text/face.cpp
    text/font_feature_settings.cpp
    text/font_library.cpp
    text/glyph_positions.cpp
    text/itemizer.cpp
    text/placement_finder.cpp
    text/properties_util.cpp
    text/renderer.cpp
    text/scrptrun.cpp
    text/symbolizer_helpers.cpp
    text/text_layout.cpp
    text/text_line.cpp
    text/text_properties.cpp

    text/formatting/base.cpp
    text/formatting/format.cpp
    text/formatting/layout.cpp
    text/formatting/list.cpp
    text/formatting/registry.cpp
    text/formatting/text.cpp

    text/placements/base.cpp
    text/placements/dummy.cpp
    text/placements/list.cpp
    text/placements/registry.cpp
    text/placements/simple.cpp
)

target_sources(mapnik PRIVATE
    util/math.cpp
    util/utf_conv_win.cpp
    util/mapped_memory_file.cpp
)

if(USE_CAIRO)
    target_sources(mapnik PRIVATE
        cairo/cairo_context.cpp
        cairo/cairo_render_vector.cpp
        cairo/cairo_renderer.cpp
        cairo/process_building_symbolizer.cpp
        cairo/process_debug_symbolizer.cpp
        cairo/process_group_symbolizer.cpp
        cairo/process_line_pattern_symbolizer.cpp
        cairo/process_line_symbolizer.cpp
        cairo/process_markers_symbolizer.cpp
        cairo/process_point_symbolizer.cpp
        cairo/process_polygon_pattern_symbolizer.cpp
        cairo/process_polygon_symbolizer.cpp
        cairo/process_raster_symbolizer.cpp
        cairo/process_text_symbolizer.cpp
    )
endif()

if(USE_JPEG)
    target_sources(mapnik PRIVATE jpeg_reader.cpp)
endif()

if(USE_PNG)
    target_sources(mapnik PRIVATE png_reader.cpp)
endif()

if(USE_TIFF)
    target_sources(mapnik PRIVATE tiff_reader.cpp)
endif()

if(USE_WEBP)
    target_sources(mapnik PRIVATE
        webp_reader.cpp
        webp_io.cpp
    )
endif()

if(USE_GRID_RENDERER)
    target_sources(mapnik PRIVATE
        grid/grid_renderer.cpp
        grid/grid.cpp
        grid/process_building_symbolizer.cpp
        grid/process_group_symbolizer.cpp
        grid/process_line_pattern_symbolizer.cpp
        grid/process_line_symbolizer.cpp
        grid/process_markers_symbolizer.cpp
        grid/process_point_symbolizer.cpp
        grid/process_polygon_pattern_symbolizer.cpp
        grid/process_polygon_symbolizer.cpp
        grid/process_raster_symbolizer.cpp
        grid/process_shield_symbolizer.cpp
        grid/process_text_symbolizer.cpp
    )
endif()

if(USE_SVG_RENDERER)
    target_sources(mapnik PRIVATE
        svg/output/process_line_symbolizer.cpp
        svg/output/process_polygon_symbolizer.cpp
        svg/output/process_symbolizers.cpp
        svg/output/svg_generator.cpp
        svg/output/svg_output_attributes.cpp
        svg/output/svg_output_grammars.cpp
        svg/output/svg_renderer.cpp
    )
endif()

if(USE_LIBXML2)
    message(STATUS "adding libxml2_loader to compilation")
    target_sources(mapnik PRIVATE libxml2_loader.cpp)
else()
    message(STATUS "adding rapidxml_loader to compilation")
    target_sources(mapnik PRIVATE rapidxml_loader.cpp)
endif()

mapnik_install(mapnik)
