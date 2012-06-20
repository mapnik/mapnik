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

// boost
#include <boost/foreach.hpp>
// mapnik
#include <mapnik/graphics.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/line_symbolizer.hpp>
#include <mapnik/vertex_converters.hpp>
// agg
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_p.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
#include "agg_renderer_outline_aa.h"
#include "agg_rasterizer_outline_aa.h"
// stl
#include <string>
#include <cmath>

namespace mapnik {


template <typename T>
void agg_renderer<T>::process(line_symbolizer const& sym,
                              mapnik::feature_impl & feature,
                              proj_transform const& prj_trans)

{
    stroke const& stroke_ = sym.get_stroke();
    color const& col = stroke_.get_color();
    unsigned r=col.red();
    unsigned g=col.green();
    unsigned b=col.blue();
    unsigned a=col.alpha();
    
    ras_ptr->reset();
    set_gamma_method(stroke_, ras_ptr);

    agg::rendering_buffer buf(current_buffer_->raw_data(),width_,height_, width_ * 4);

    typedef agg::rgba8 color_type;
    typedef agg::order_rgba order_type;
    typedef agg::pixel32_type pixel_type;
    typedef agg::comp_op_adaptor_rgba_pre<color_type, order_type> blender_type; // comp blender
    typedef agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixfmt_comp_type;
    typedef agg::renderer_base<pixfmt_comp_type> renderer_base;

    pixfmt_comp_type pixf(buf);
    pixf.comp_op(static_cast<agg::comp_op_e>(sym.comp_op()));
    renderer_base renb(pixf);
    
    agg::trans_affine tr;
    evaluate_transform(tr, feature, sym.get_transform());

    box2d<double> ext = query_extent_ * 1.1;

    if (sym.get_rasterizer() == RASTERIZER_FAST)
    {
        typedef agg::renderer_outline_aa<renderer_base> renderer_type;
        typedef agg::rasterizer_outline_aa<renderer_type> rasterizer_type;
        // need to reduce width by half to match standard rasterizer look
        double scaled = scale_factor_ * .5;
        agg::line_profile_aa profile(stroke_.get_width() * scaled, agg::gamma_power(stroke_.get_gamma()));
        renderer_type ren(renb, profile);
        ren.color(agg::rgba8_pre(r, g, b, int(a*stroke_.get_opacity())));
        rasterizer_type ras(ren);
        set_join_caps_aa(stroke_,ras);

        typedef boost::mpl::vector<clip_line_tag,transform_tag, offset_transform_tag, affine_transform_tag, smooth_tag, dash_tag, stroke_tag> conv_types;
        vertex_converter<box2d<double>, rasterizer_type, line_symbolizer,
                         CoordTransform, proj_transform, agg::trans_affine, conv_types>
            converter(ext,ras,sym,t_,prj_trans,tr,scaled);

        if (sym.clip()) converter.set<clip_line_tag>(); // optional clip (default: true)
        converter.set<transform_tag>(); // always transform
        if (fabs(sym.offset()) > 0.0) converter.set<offset_transform_tag>(); // parallel offset
        converter.set<affine_transform_tag>(); // optional affine transform
        if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter
        if (stroke_.has_dash()) converter.set<dash_tag>();
        converter.set<stroke_tag>(); //always stroke

        BOOST_FOREACH( geometry_type & geom, feature.paths())
        {
            if (geom.num_points() > 1)
            {
                converter.apply(geom);
            }
        }
    }
    else
    {
        typedef boost::mpl::vector<clip_line_tag,transform_tag, offset_transform_tag, affine_transform_tag, smooth_tag, dash_tag, stroke_tag> conv_types;
        vertex_converter<box2d<double>, rasterizer, line_symbolizer,
                         CoordTransform, proj_transform, agg::trans_affine, conv_types>
            converter(ext,*ras_ptr,sym,t_,prj_trans,tr,scale_factor_);

        if (sym.clip()) converter.set<clip_line_tag>(); // optional clip (default: true)
        converter.set<transform_tag>(); // always transform
        if (fabs(sym.offset()) > 0.0) converter.set<offset_transform_tag>(); // parallel offset
        converter.set<affine_transform_tag>(); // optional affine transform
        if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter
        if (stroke_.has_dash()) converter.set<dash_tag>();
        converter.set<stroke_tag>(); //always stroke

        BOOST_FOREACH( geometry_type & geom, feature.paths())
        {
            if (geom.num_points() > 1)
            {
                converter.apply(geom);
            }
        }

        typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_type;
        renderer_base renb(pixf);
        renderer_type ren(renb);
        ren.color(agg::rgba8_pre(r, g, b, int(a * stroke_.get_opacity())));
        agg::scanline_u8 sl;
        agg::render_scanlines(*ras_ptr, sl, ren);
    }
}


template void agg_renderer<image_32>::process(line_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);

}
