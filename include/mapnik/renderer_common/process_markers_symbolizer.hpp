/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#ifndef MAPNIK_RENDERER_COMMON_PROCESS_MARKERS_SYMBOLIZER_HPP
#define MAPNIK_RENDERER_COMMON_PROCESS_MARKERS_SYMBOLIZER_HPP

#include <mapnik/renderer_common.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/marker_helpers.hpp>

namespace mapnik {

template <typename T0, typename T1, typename T2>
void render_markers_symbolizer(markers_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans,
                               renderer_common const& common,
                               box2d<double> const& clip_box,
                               T2 const& renderer_context)
{
    using namespace mapnik::svg;
    using vector_dispatch_type = T0;
    using raster_dispatch_type = T1;

    std::string filename = get<std::string>(sym, keys::file, feature, common.vars_, "shape://ellipse");
    bool clip = get<value_bool, keys::clip>(sym, feature, common.vars_);
    double offset = get<value_double, keys::offset>(sym, feature, common.vars_);
    double simplify_tolerance = get<value_double, keys::simplify_tolerance>(sym, feature, common.vars_);
    double smooth = get<value_double, keys::smooth>(sym, feature, common.vars_);

    // https://github.com/mapnik/mapnik/issues/1316
    bool snap_to_pixels = !mapnik::marker_cache::instance().is_uri(filename);
    if (!filename.empty())
    {
        boost::optional<marker_ptr> mark = mapnik::marker_cache::instance().find(filename, true);
        if (mark && *mark)
        {
            agg::trans_affine geom_tr;
            auto transform = get_optional<transform_type>(sym, keys::geometry_transform);
            if (transform) evaluate_transform(geom_tr, feature, common.vars_, *transform, common.scale_factor_);
            agg::trans_affine image_tr = agg::trans_affine_scaling(common.scale_factor_);

            if ((*mark)->is_vector())
            {
                boost::optional<svg_path_ptr> const& stock_vector_marker = (*mark)->get_vector_data();

                // special case for simple ellipse markers
                // to allow for full control over rx/ry dimensions
                if (filename == "shape://ellipse"
                   && (has_key(sym,keys::width) || has_key(sym,keys::height)))
                {
                    svg_storage_type marker_ellipse;
                    vertex_stl_adapter<svg_path_storage> stl_storage(marker_ellipse.source());
                    svg_path_adapter svg_path(stl_storage);
                    build_ellipse(sym, feature, common.vars_, marker_ellipse, svg_path);
                    svg_attribute_type attributes;
                    bool result = push_explicit_style( (*stock_vector_marker)->attributes(), attributes, sym, feature, common.vars_);
                    auto image_transform = get_optional<transform_type>(sym, keys::image_transform);
                    if (image_transform) evaluate_transform(image_tr, feature, common.vars_, *image_transform);
                    box2d<double> bbox = marker_ellipse.bounding_box();
                    vector_dispatch_type rasterizer_dispatch(svg_path,
                                                             result ? attributes : (*stock_vector_marker)->attributes(),
                                                             bbox,
                                                             image_tr,
                                                             sym,
                                                             *common.detector_,
                                                             common.scale_factor_,
                                                             feature,
                                                             common.vars_,
                                                             snap_to_pixels,
                                                             renderer_context);

                    using vertex_converter_type = vertex_converter<vector_dispatch_type,clip_line_tag,
                                                  clip_poly_tag,
                                                  transform_tag,
                                                  affine_transform_tag,
                                                  simplify_tag, smooth_tag,
                                                  offset_transform_tag>;
                    vertex_converter_type converter(clip_box, rasterizer_dispatch, sym,common.t_,prj_trans,geom_tr,feature,common.vars_,common.scale_factor_);
                    if (clip && feature.paths().size() > 0) // optional clip (default: true)
                    {
                        geometry_type::types type = feature.paths()[0].type();
                        if (type == geometry_type::types::Polygon)
                            converter.template set<clip_poly_tag>();
                        else if (type == geometry_type::types::LineString)
                            converter.template set<clip_line_tag>();
                    }
                    converter.template set<transform_tag>(); //always transform
                    if (std::fabs(offset) > 0.0) converter.template set<offset_transform_tag>(); // parallel offset
                    converter.template set<affine_transform_tag>(); // optional affine transform
                    if (simplify_tolerance > 0.0) converter.template set<simplify_tag>(); // optional simplify converter
                    if (smooth > 0.0) converter.template set<smooth_tag>(); // optional smooth converter
                    apply_markers_multi(feature, common.vars_, converter, sym);
                }
                else
                {
                    box2d<double> const& bbox = (*mark)->bounding_box();
                    setup_transform_scaling(image_tr, bbox.width(), bbox.height(), feature, common.vars_, sym);
                    auto image_transform = get_optional<transform_type>(sym, keys::image_transform);
                    if (image_transform) evaluate_transform(image_tr, feature, common.vars_, *image_transform);
                    vertex_stl_adapter<svg_path_storage> stl_storage((*stock_vector_marker)->source());
                    svg_path_adapter svg_path(stl_storage);
                    svg_attribute_type attributes;
                    bool result = push_explicit_style( (*stock_vector_marker)->attributes(), attributes, sym, feature, common.vars_);
                    vector_dispatch_type rasterizer_dispatch(svg_path,
                                                             result ? attributes : (*stock_vector_marker)->attributes(),
                                                             bbox,
                                                             image_tr,
                                                             sym,
                                                             *common.detector_,
                                                             common.scale_factor_,
                                                             feature,
                                                             common.vars_,
                                                             snap_to_pixels,
                                                             renderer_context);

                    using vertex_converter_type = vertex_converter<vector_dispatch_type,clip_line_tag,
                                                  clip_poly_tag,
                                                  transform_tag,
                                                  affine_transform_tag,
                                                  simplify_tag, smooth_tag,
                                                  offset_transform_tag>;
                    vertex_converter_type converter(clip_box, rasterizer_dispatch, sym,common.t_,prj_trans,geom_tr,feature,common.vars_,common.scale_factor_);
                    if (clip && feature.paths().size() > 0) // optional clip (default: true)
                    {
                        geometry_type::types type = feature.paths()[0].type();
                        if (type == geometry_type::types::Polygon)
                            converter.template set<clip_poly_tag>();
                        else if (type == geometry_type::types::LineString)
                            converter.template set<clip_line_tag>();
                    }
                    converter.template set<transform_tag>(); //always transform
                    if (std::fabs(offset) > 0.0) converter.template set<offset_transform_tag>(); // parallel offset
                    converter.template set<affine_transform_tag>(); // optional affine transform
                    if (simplify_tolerance > 0.0) converter.template set<simplify_tag>(); // optional simplify converter
                    if (smooth > 0.0) converter.template set<smooth_tag>(); // optional smooth converter
                    apply_markers_multi(feature, common.vars_, converter, sym);
                }
            }
            else // raster markers
            {
                setup_transform_scaling(image_tr, (*mark)->width(), (*mark)->height(), feature, common.vars_, sym);
                auto image_transform = get_optional<transform_type>(sym, keys::image_transform);
                if (image_transform) evaluate_transform(image_tr, feature, common.vars_, *image_transform);
                box2d<double> const& bbox = (*mark)->bounding_box();
                boost::optional<mapnik::image_ptr> marker = (*mark)->get_bitmap_data();
                // - clamp sizes to > 4 pixels of interactivity
                coord2d center = bbox.center();
                agg::trans_affine_translation recenter(-center.x, -center.y);
                agg::trans_affine marker_trans = recenter * image_tr;
                raster_dispatch_type rasterizer_dispatch(**marker,
                                                         marker_trans,
                                                         sym,
                                                         *common.detector_,
                                                         common.scale_factor_,
                                                         feature,
                                                         common.vars_,
                                                         renderer_context);

                using vertex_converter_type = vertex_converter<raster_dispatch_type,clip_line_tag,
                                              clip_poly_tag,
                                              transform_tag,
                                              affine_transform_tag,
                                              simplify_tag, smooth_tag,
                                              offset_transform_tag>;
                vertex_converter_type converter(clip_box, rasterizer_dispatch, sym,common.t_,prj_trans,geom_tr,feature,common.vars_,common.scale_factor_);

                if (clip && feature.paths().size() > 0) // optional clip (default: true)
                {
                    geometry_type::types type = feature.paths()[0].type();
                    if (type == geometry_type::types::Polygon)
                        converter.template set<clip_poly_tag>();
                    else if (type == geometry_type::types::LineString)
                        converter.template set<clip_line_tag>();
                }
                converter.template set<transform_tag>(); //always transform
                if (std::fabs(offset) > 0.0) converter.template set<offset_transform_tag>(); // parallel offset
                converter.template set<affine_transform_tag>(); // optional affine transform
                if (simplify_tolerance > 0.0) converter.template set<simplify_tag>(); // optional simplify converter
                if (smooth > 0.0) converter.template set<smooth_tag>(); // optional smooth converter
                apply_markers_multi(feature, common.vars_, converter, sym);
            }
        }
    }
}

} // namespace mapnik

#endif // MAPNIK_RENDERER_COMMON_PROCESS_MARKERS_SYMBOLIZER_HPP
