/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#if defined(HAVE_CAIRO)

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/cairo/cairo_render_vector.hpp>
#include <mapnik/renderer_common/render_pattern.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/renderer_common/clipping_extent.hpp>

namespace mapnik
{

template <typename T>
void cairo_renderer<T>::process(polygon_pattern_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);
    std::string filename = get<std::string>(sym, keys::file, feature, common_.vars_);
    bool clip = get<bool>(sym, keys::clip, feature, common_.vars_, false);
    double simplify_tolerance = get<double>(sym, keys::simplify_tolerance, feature, common_.vars_, 0.0);
    double smooth = get<double>(sym, keys::smooth, feature, common_.vars_, 0.0);
    double opacity = get<double>(sym,keys::opacity, feature, common_.vars_, 1.0);
    agg::trans_affine image_tr = agg::trans_affine_scaling(common_.scale_factor_);
    auto image_transform = get_optional<transform_type>(sym, keys::image_transform);
    if (image_transform) evaluate_transform(image_tr, feature, common_.vars_, *image_transform);

    cairo_save_restore guard(context_);
    context_.set_operator(comp_op);

    boost::optional<mapnik::marker_ptr> marker = mapnik::marker_cache::instance().find(filename,true);
    if (!marker || !(*marker)) return;

    unsigned offset_x=0;
    unsigned offset_y=0;
    box2d<double> const& clip_box = clipping_extent(common_);
    pattern_alignment_enum alignment = get<pattern_alignment_enum>(sym, keys::alignment, feature, common_.vars_, GLOBAL_ALIGNMENT);
    if (alignment == LOCAL_ALIGNMENT)
    {
        double x0 = 0.0;
        double y0 = 0.0;

        if (feature.num_geometries() > 0)
        {
            using clipped_geometry_type = agg::conv_clip_polygon<geometry_type>;
            using path_type = transform_path_adapter<view_transform,clipped_geometry_type>;
            clipped_geometry_type clipped(feature.get_geometry(0));
            clipped.clip_box(clip_box.minx(), clip_box.miny(),
                             clip_box.maxx(), clip_box.maxy());
            path_type path(common_.t_, clipped, prj_trans);
            path.vertex(&x0, &y0);
        }
        offset_x = std::abs(clip_box.width() - x0);
        offset_y = std::abs(clip_box.height() - y0);
    }

    if ((*marker)->is_bitmap())
    {
        cairo_pattern pattern(**((*marker)->get_bitmap_data()), opacity);
        pattern.set_extend(CAIRO_EXTEND_REPEAT);
        pattern.set_origin(offset_x, offset_y);
        context_.set_pattern(pattern);
    }
    else
    {
        mapnik::rasterizer ras;
        image_ptr image = render_pattern(ras, **marker, image_tr, 1.0); //
        cairo_pattern pattern(*image, opacity);
        pattern.set_extend(CAIRO_EXTEND_REPEAT);
        pattern.set_origin(offset_x, offset_y);
        context_.set_pattern(pattern);
    }

    agg::trans_affine tr;
    auto geom_transform = get_optional<transform_type>(sym, keys::geometry_transform);
    if (geom_transform) { evaluate_transform(tr, feature, common_.vars_, *geom_transform, common_.scale_factor_); }

    using conv_types = boost::mpl::vector<clip_poly_tag,transform_tag,affine_transform_tag,simplify_tag,smooth_tag>;
    vertex_converter<cairo_context, conv_types>
        converter(clip_box, context_,sym,common_.t_,prj_trans,tr,feature,common_.vars_,common_.scale_factor_);

    if (prj_trans.equal() && clip) converter.set<clip_poly_tag>(); //optional clip (default: true)
    converter.set<transform_tag>(); //always transform
    converter.set<affine_transform_tag>();
    if (simplify_tolerance > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (smooth > 0.0) converter.set<smooth_tag>(); // optional smooth converter

    for ( geometry_type & geom : feature.paths())
    {
        if (geom.size() > 2)
        {
            converter.apply(geom);
        }
    }
    // fill polygon
    context_.set_fill_rule(CAIRO_FILL_RULE_EVEN_ODD);
    context_.fill();
}

template void cairo_renderer<cairo_ptr>::process(polygon_pattern_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

}

#endif // HAVE_CAIRO
