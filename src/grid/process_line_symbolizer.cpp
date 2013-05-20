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

#include <mapnik/line_symbolizer.hpp>
#include <mapnik/vertex_converters.hpp>

// agg
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_bin.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"

// boost
#include <boost/foreach.hpp>

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

    grid_rendering_buffer buf(pixmap_.raw_data(), width_, height_, width_);
    pixfmt_type pixf(buf);

    grid_renderer_base_type renb(pixf);
    renderer_type ren(renb);

    ras_ptr->reset();

    stroke const& stroke_ = sym.get_stroke();

    agg::trans_affine tr;
    evaluate_transform(tr, feature, sym.get_transform());

    box2d<double> clipping_extent = query_extent_;
    if (sym.clip())
    {
        double padding = (double)(query_extent_.width()/pixmap_.width());
        double half_stroke = stroke_.get_width()/2.0;
        if (half_stroke > 1)
            padding *= half_stroke;
        if (std::fabs(sym.offset()) > 0)
            padding *= std::fabs(sym.offset()) * 1.2;
        padding *= scale_factor_;
        clipping_extent.pad(padding);
    }

    vertex_converter<box2d<double>, grid_rasterizer, line_symbolizer,
                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
        converter(clipping_extent,*ras_ptr,sym,t_,prj_trans,tr,scale_factor_);
    if (sym.clip()) converter.set<clip_line_tag>(); // optional clip (default: true)
    converter.set<transform_tag>(); // always transform
    if (std::fabs(sym.offset()) > 0.0) converter.set<offset_transform_tag>(); // parallel offset
    converter.set<affine_transform_tag>(); // optional affine transform
    if (sym.simplify_tolerance() > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter
    if (stroke_.has_dash()) converter.set<dash_tag>();
    converter.set<stroke_tag>(); //always stroke

    BOOST_FOREACH( geometry_type & geom, feature.paths())
    {
        if (geom.size() > 1)
        {
            converter.apply(geom);
        }
    }

    // render id
    ren.color(color_type(feature.id()));
    agg::render_scanlines(*ras_ptr, sl, ren);

    // add feature properties to grid cache
    pixmap_.add_feature(feature);

}


template void grid_renderer<grid>::process(line_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);

}

