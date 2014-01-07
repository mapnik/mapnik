/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_renderer_base.hpp>
#include <mapnik/grid/grid.hpp>

#include <mapnik/vertex_converters.hpp>

// agg
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_bin.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"

// boost


// stl
#include <string>

namespace mapnik {

template <typename T>
void grid_renderer<T>::process(line_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans)
{
    typedef typename grid_renderer_base_type::pixfmt_type pixfmt_type;
    typedef typename grid_renderer_base_type::pixfmt_type::color_type color_type;
    typedef agg::renderer_scanline_bin_solid<grid_renderer_base_type> renderer_type;
    typedef boost::mpl::vector<clip_line_tag, transform_tag,
                               offset_transform_tag, affine_transform_tag,
                               simplify_tag, smooth_tag, dash_tag, stroke_tag> conv_types;
    agg::scanline_bin sl;

    grid_rendering_buffer buf(pixmap_.raw_data(), common_.width_, common_.height_, common_.width_);
    pixfmt_type pixf(buf);

    grid_renderer_base_type renb(pixf);
    renderer_type ren(renb);

    ras_ptr->reset();

    agg::trans_affine tr;
    auto transform = get_optional<transform_type>(sym, keys::geometry_transform);
    if (transform) { evaluate_transform(tr, feature, *transform); }

    box2d<double> clipping_extent = common_.query_extent_;

    bool clip = get<value_bool>(sym, keys::clip, feature, true);
    double width = get<value_double>(sym, keys::stroke_width, feature, 1.0);
    double offset = get<value_double>(sym, keys::offset, feature, 0.0);
    double simplify_tolerance = get<value_double>(sym, keys::simplify_tolerance, feature, 0.0);
    double smooth = get<value_double>(sym, keys::smooth, feature, false);
    bool has_dash = has_key<dash_array>(sym, keys::stroke_dasharray);

    if (clip)
    {
        double padding = (double)(common_.query_extent_.width()/pixmap_.width());
        double half_stroke = width/2.0;
        if (half_stroke > 1)
            padding *= half_stroke;
        if (std::fabs(offset) > 0)
            padding *= std::fabs(offset) * 1.2;
        padding *= common_.scale_factor_;
        clipping_extent.pad(padding);
    }

    vertex_converter<box2d<double>, grid_rasterizer, line_symbolizer,
                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
        converter(clipping_extent,*ras_ptr,sym,common_.t_,prj_trans,tr,common_.scale_factor_);
    if (clip) converter.set<clip_line_tag>(); // optional clip (default: true)
    converter.set<transform_tag>(); // always transform
    if (std::fabs(offset) > 0.0) converter.set<offset_transform_tag>(); // parallel offset
    converter.set<affine_transform_tag>(); // optional affine transform
    if (simplify_tolerance > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (smooth > 0.0) converter.set<smooth_tag>(); // optional smooth converter
    if (has_dash) converter.set<dash_tag>();
    converter.set<stroke_tag>(); //always stroke

    for ( geometry_type & geom : feature.paths())
    {
        if (geom.size() > 1)
        {
            converter.apply(geom);
        }
    }

    // render id
    ren.color(color_type(feature.id()));
    ras_ptr->filling_rule(agg::fill_non_zero);
    agg::render_scanlines(*ras_ptr, sl, ren);

    // add feature properties to grid cache
    pixmap_.add_feature(feature);

}


template void grid_renderer<grid>::process(line_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);

}
