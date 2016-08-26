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
#include <mapnik/feature.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/polygon_symbolizer.hpp>
#include <mapnik/vertex_converters.hpp>

// agg
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_renderer_scanline.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"

namespace mapnik {

template <typename T>
void agg_renderer<T>::process(polygon_symbolizer const& sym,
                              mapnik::feature_impl & feature,
                              proj_transform const& prj_trans)
{

    ras_ptr->reset();
    if (sym.get_gamma() != gamma_ || sym.get_gamma_method() != gamma_method_)
    {
        set_gamma_method(sym, ras_ptr);
        gamma_method_ = sym.get_gamma_method();
        gamma_ = sym.get_gamma();
    }
    agg::trans_affine tr;
    evaluate_transform(tr, feature, sym.get_transform(), scale_factor_);
    box2d<double> clip_box = clipping_extent();
    typedef boost::mpl::vector<clip_poly_tag,transform_tag,affine_transform_tag,simplify_tag,smooth_tag> conv_types;
    vertex_converter<box2d<double>, rasterizer, polygon_symbolizer,
                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
        converter(clip_box,*ras_ptr,sym,t_,prj_trans,tr,scale_factor_);

    if (prj_trans.equal() && sym.clip()) converter.set<clip_poly_tag>(); //optional clip (default: true)
    converter.set<transform_tag>(); //always transform
    converter.set<affine_transform_tag>();
    if (sym.simplify_tolerance() > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter

    BOOST_FOREACH( geometry_type & geom, feature.paths())
    {
        if (geom.size() > 2)
        {
            converter.apply(geom);
        }
    }

    agg::rendering_buffer buf(current_buffer_->raw_data(),current_buffer_->width(),current_buffer_->height(), current_buffer_->width() * 4);

    color const& fill = sym.get_fill();
    unsigned r=fill.red();
    unsigned g=fill.green();
    unsigned b=fill.blue();
    unsigned a=fill.alpha();

    typedef agg::rgba8 color_type;
    typedef agg::order_rgba order_type;
    typedef agg::comp_op_adaptor_rgba_pre<color_type, order_type> blender_type; // comp blender
    typedef agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixfmt_comp_type;
    typedef agg::renderer_base<pixfmt_comp_type> renderer_base;
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_type;
    pixfmt_comp_type pixf(buf);
    pixf.comp_op(static_cast<agg::comp_op_e>(sym.comp_op()));
    renderer_base renb(pixf);
    renderer_type ren(renb);
    ren.color(agg::rgba8_pre(r, g, b, int(a * sym.get_opacity())));
    agg::scanline_u8 sl;
    ras_ptr->filling_rule(agg::fill_even_odd);
    agg::render_scanlines(*ras_ptr, sl, ren);

}

template void agg_renderer<image_32>::process(polygon_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);

}
