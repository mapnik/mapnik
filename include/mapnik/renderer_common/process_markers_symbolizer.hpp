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

#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>

namespace mapnik {

namespace {



} // anonymous namespace

template <typename F1, typename F2>
void render_markers_symbolizer(markers_symbolizer const &sym,
                               mapnik::feature_impl &feature,
                               proj_transform const &prj_trans,
                               renderer_common &common,
                               box2d<double> const &clip_box,
                               F1 make_vector_dispatch,
                               F2 make_raster_dispatch)
{
    using namespace mapnik::svg;
    typedef boost::mpl::vector<clip_poly_tag,transform_tag,smooth_tag> conv_types;
    typedef agg::pod_bvector<path_attributes> svg_attribute_type;

    std::string filename = get<std::string>(sym, keys::file, feature, "shape://ellipse");
    bool clip = get<value_bool>(sym, keys::clip, feature, false);
    double smooth = get<value_double>(sym, keys::smooth, feature, false);

    // https://github.com/mapnik/mapnik/issues/1316
    bool snap_pixels = !mapnik::marker_cache::instance().is_uri(filename);
    if (!filename.empty())
    {
        boost::optional<marker_ptr> mark = mapnik::marker_cache::instance().find(filename, true);
        if (mark && *mark)
        {
            agg::trans_affine tr = agg::trans_affine_scaling(common.scale_factor_);

            if ((*mark)->is_vector())
            {
                boost::optional<svg_path_ptr> const& stock_vector_marker = (*mark)->get_vector_data();

                auto width_expr = get_optional<expression_ptr>(sym, keys::width);
                auto height_expr = get_optional<expression_ptr>(sym, keys::height);

                // special case for simple ellipse markers
                // to allow for full control over rx/ry dimensions
                if (filename == "shape://ellipse"
                   && (width_expr || height_expr))
                {
                    svg_storage_type marker_ellipse;
                    vertex_stl_adapter<svg_path_storage> stl_storage(marker_ellipse.source());
                    svg_path_adapter svg_path(stl_storage);
                    build_ellipse(sym, feature, marker_ellipse, svg_path);
                    svg_attribute_type attributes;
                    bool result = push_explicit_style( (*stock_vector_marker)->attributes(), attributes, sym);
                    auto image_transform = get_optional<transform_type>(sym, keys::image_transform);
                    if (image_transform) evaluate_transform(tr, feature, *image_transform);
                    box2d<double> bbox = marker_ellipse.bounding_box();

                    auto rasterizer_dispatch = make_vector_dispatch(
                      svg_path, result ? attributes : (*stock_vector_marker)->attributes(), 
                      marker_ellipse, bbox, tr, snap_pixels);
                    typedef decltype(rasterizer_dispatch) dispatch_type;

                    vertex_converter<box2d<double>, dispatch_type, markers_symbolizer,
                                     CoordTransform, proj_transform, agg::trans_affine, conv_types, feature_impl>
                        converter(clip_box, rasterizer_dispatch, sym,common.t_,prj_trans,tr,feature,common.scale_factor_);
                    if (clip && feature.paths().size() > 0) // optional clip (default: true)
                    {
                        geometry_type::types type = feature.paths()[0].type();
                        if (type == geometry_type::types::Polygon)
                            converter.template set<clip_poly_tag>();
                        // line clipping disabled due to https://github.com/mapnik/mapnik/issues/1426
                        //else if (type == LineString)
                        //    converter.template set<clip_line_tag>();
                        // don't clip if type==Point
                    }
                    converter.template set<transform_tag>(); //always transform
                    if (smooth > 0.0) converter.template set<smooth_tag>(); // optional smooth converter
                    apply_markers_multi(feature, converter, sym);
                }
                else
                {
                    box2d<double> const& bbox = (*mark)->bounding_box();
                    setup_transform_scaling(tr, bbox.width(), bbox.height(), feature, sym);
                    auto image_transform = get_optional<transform_type>(sym, keys::image_transform);
                    if (image_transform) evaluate_transform(tr, feature, *image_transform);
                    vertex_stl_adapter<svg_path_storage> stl_storage((*stock_vector_marker)->source());
                    svg_path_adapter svg_path(stl_storage);
                    svg_attribute_type attributes;
                    bool result = push_explicit_style( (*stock_vector_marker)->attributes(), attributes, sym);
                    auto rasterizer_dispatch = make_vector_dispatch(
                      svg_path, result ? attributes : (*stock_vector_marker)->attributes(),
                      **stock_vector_marker, bbox, tr, snap_pixels);
                    typedef decltype(rasterizer_dispatch) dispatch_type;

                    vertex_converter<box2d<double>, dispatch_type, markers_symbolizer,
                                     CoordTransform, proj_transform, agg::trans_affine, conv_types, feature_impl>
                        converter(clip_box, rasterizer_dispatch, sym,common.t_,prj_trans,tr,feature,common.scale_factor_);
                    if (clip && feature.paths().size() > 0) // optional clip (default: true)
                    {
                        geometry_type::types type = feature.paths()[0].type();
                        if (type == geometry_type::types::Polygon)
                            converter.template set<clip_poly_tag>();
                        // line clipping disabled due to https://github.com/mapnik/mapnik/issues/1426
                        //else if (type == LineString)
                        //    converter.template set<clip_line_tag>();
                        // don't clip if type==Point
                    }
                    converter.template set<transform_tag>(); //always transform
                    if (smooth > 0.0) converter.template set<smooth_tag>(); // optional smooth converter
                    apply_markers_multi(feature, converter, sym);
                }
            }
            else // raster markers
            {
                setup_transform_scaling(tr, (*mark)->width(), (*mark)->height(), feature, sym);
                auto image_transform = get_optional<transform_type>(sym, keys::image_transform);
                if (image_transform) evaluate_transform(tr, feature, *image_transform);
                box2d<double> const& bbox = (*mark)->bounding_box();
                boost::optional<mapnik::image_ptr> marker = (*mark)->get_bitmap_data();
                
                auto rasterizer_dispatch = make_raster_dispatch(**marker, tr, bbox);
                typedef decltype(rasterizer_dispatch) dispatch_type;

                vertex_converter<box2d<double>, dispatch_type, markers_symbolizer,
                                 CoordTransform, proj_transform, agg::trans_affine, conv_types, feature_impl>
                    converter(clip_box, rasterizer_dispatch, sym,common.t_,prj_trans,tr,feature,common.scale_factor_);

                if (clip && feature.paths().size() > 0) // optional clip (default: true)
                {
                    geometry_type::types type = feature.paths()[0].type();
                    if (type == geometry_type::types::Polygon)
                        converter.template set<clip_poly_tag>();
                    // line clipping disabled due to https://github.com/mapnik/mapnik/issues/1426
                    //else if (type == geometry_type::types::LineString)
                    //    converter.template set<clip_line_tag>();
                    // don't clip if type==geometry_type::types::Point
                }
                converter.template set<transform_tag>(); //always transform
                if (smooth > 0.0) converter.template set<smooth_tag>(); // optional smooth converter
                apply_markers_multi(feature, converter, sym);
            }
        }
    }
}

} // namespace mapnik

#endif /* MAPNIK_RENDERER_COMMON_PROCESS_MARKERS_SYMBOLIZER_HPP */
