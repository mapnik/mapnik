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
#include <mapnik/image_compositing.hpp>
#include <mapnik/image_data.hpp>

// boost
#include <boost/assign/list_of.hpp>
#include <boost/bimap.hpp>

// agg
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_pixfmt_rgba.h"

namespace mapnik
{

typedef boost::bimap<composite_mode_e, std::string> comp_op_lookup_type;
static const comp_op_lookup_type comp_lookup = boost::assign::list_of<comp_op_lookup_type::relation>
    (clear,"clear")
    (src,"src")
    (dst,"dst")
    (src_over,"dst")
    (dst_over,"dst_over")
    (src_in,"src_in")
    (dst_in,"dst_in")
    (src_out,"src_out")
    (dst_out,"dst_out")
    (src_atop,"src_atop")
    (dst_atop,"dst_atop")
    (_xor,"xor")
    (plus,"plus")
    (minus,"minus")
    (multiply,"multiply")
    (screen,"screen")
    (overlay,"overlay")
    (darken,"darken")
    (lighten,"lighten")
    (color_dodge,"color_dodge")
    (color_burn,"color_burn")
    (hard_light,"hard_light")
    (soft_light,"soft_light")
    (difference,"difference")
    (exclusion,"exclusion")
    (contrast,"contrast")
    (invert,"invert")
    (invert_rgb,"invert_rgb");

composite_mode_e comp_op_from_string(std::string const& name)
{
    comp_op_lookup_type::right_const_iterator right_iter = comp_lookup.right.find(name);
    if (right_iter != comp_lookup.right.end())
    {
        return right_iter->second;
    }
    return clear;
}

template <typename T1, typename T2>
void composite(T1 & im, T2 & im2, composite_mode_e mode)
{
    typedef agg::rgba8 color;
    typedef agg::order_rgba order;
    typedef agg::pixel32_type pixel_type;
    typedef agg::comp_op_adaptor_rgba<color, order> blender_type;
    typedef agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixfmt_type;
    typedef agg::renderer_base<pixfmt_type> renderer_type;

    agg::rendering_buffer source(im.getBytes(),im.width(),im.height(),im.width() * 4);
    agg::rendering_buffer mask(im2.getBytes(),im2.width(),im2.height(),im2.width() * 4);

    agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixf(source);
    pixf.premultiply();
    agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixf_mask(mask);
    pixf.comp_op(static_cast<agg::comp_op_e>(mode));
    
    renderer_type ren(pixf);
    agg::renderer_base<pixfmt_type> rb(pixf);
    rb.blend_from(pixf_mask,0,0,0,255);
}


template void composite<mapnik::image_data_32,mapnik::image_data_32>(mapnik::image_data_32 & im, mapnik::image_data_32 & im2, composite_mode_e mode);

}
