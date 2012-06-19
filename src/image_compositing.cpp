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
    (src_over,"src-over")
    (dst_over,"dst-over")
    (src_in,"src-in")
    (dst_in,"dst-in")
    (src_out,"src-out")
    (dst_out,"dst-out")
    (src_atop,"src-atop")
    (dst_atop,"dst-atop")
    (_xor,"xor")
    (plus,"plus")
    (minus,"minus")
    (multiply,"multiply")
    (screen,"screen")
    (overlay,"overlay")
    (darken,"darken")
    (lighten,"lighten")
    (color_dodge,"color-dodge")
    (color_burn,"color-burn")
    (hard_light,"hard-light")
    (soft_light,"soft-light")
    (difference,"difference")
    (exclusion,"exclusion")
    (contrast,"contrast")
    (invert,"invert")
    (invert_rgb,"invert-rgb")
    (grain_merge,"grain-merge")
    (grain_extract,"grain-extract")
    (hue,"hue")
    (saturation,"saturation")
    (_color,"color")
    (_value,"value")
    (colorize_alpha,"colorize-alpha")
    ;

boost::optional<composite_mode_e> comp_op_from_string(std::string const& name)
{
    boost::optional<composite_mode_e> mode;
    comp_op_lookup_type::right_const_iterator right_iter = comp_lookup.right.find(name);
    if (right_iter != comp_lookup.right.end())
    {
        mode.reset(right_iter->second);
    }
    return mode;
}

template <typename T1, typename T2>
void composite(T1 & dst, T2 & src, composite_mode_e mode,
               float opacity,
               int dx,
               int dy,
               bool premultiply_src)
{
    typedef agg::rgba8 color;
    typedef agg::order_rgba order;
    typedef agg::comp_op_adaptor_rgba_pre<color, order> blender_type;
    typedef agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixfmt_type;
    typedef agg::renderer_base<pixfmt_type> renderer_type;

    agg::rendering_buffer dst_buffer(dst.getBytes(),dst.width(),dst.height(),dst.width() * 4);
    agg::rendering_buffer src_buffer(src.getBytes(),src.width(),src.height(),src.width() * 4);

    pixfmt_type pixf(dst_buffer);
    pixf.comp_op(static_cast<agg::comp_op_e>(mode));
    
    agg::pixfmt_rgba32 pixf_mask(src_buffer);
    if (premultiply_src)  pixf_mask.premultiply();
    renderer_type ren(pixf);
    ren.blend_from(pixf_mask,0,dx,dy,unsigned(255*opacity));
}

template void composite<mapnik::image_data_32,mapnik::image_data_32>(mapnik::image_data_32&, mapnik::image_data_32& ,composite_mode_e, float, int, int, bool);

}
