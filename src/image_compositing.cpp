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
#include "agg_color_rgba.h"

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

boost::optional<std::string> comp_op_to_string(composite_mode_e comp_op)
{
    boost::optional<std::string> mode;
    comp_op_lookup_type::left_const_iterator left_iter = comp_lookup.left.find(comp_op);
    if (left_iter != comp_lookup.left.end())
    {
        mode.reset(left_iter->second);
    }
    return mode;
}

/*
Note: the difference between agg::pixfmt_rgba32 and agg:pixfmt_rgba32_pre is subtle.

From http://www.antigrain.com/news/release_notes/v22.agdoc.html:

Format agg::pixfmt_rgba32 is the main and the fastest pixel format and it's supposed to be used in most cases. But it always uses plain colors as input and produces pre-multiplied result on the canvas. It has even less number of calculations than agg::pixfmt_rgba32_pre. Format agg::pixfmt_rgba32_plain is slow because of division operations. APIs allowing for alpha-blending require premultiplied colors. Besides, if you display RGBA with RGB API (that is, without alpha, like WinAPI BitBlt), the colors still must be premultiplied. Note that the formulas in agg::pixfmt_rgba32 and agg::pixfmt_rgb24 are exactly the same! So, premultiplied colors are more natural and agg::pixfmt_rgba32_plain is rather useless.

Format agg::pixfmt_rgba32_pre is a bit slower than agg::pixfmt_rgba32 because of additional "cover" values, i.e. secondary alphas, that are to be mixed with the source premultiplied color. That spoils the beauty of the premultiplied colors idea. But the "cover" values are important because there can be other color spaces and color types that don't have any "alpha" at all, or the alpha is incompatible with integral types. So, the "cover" is a secondary, uniform alpha in range of 0…255, used specifically for anti-aliasing purposes. 
One needs to consider this issue when transforming images. Actually, all RGBA images are supposed to be in the premultiplied color space and the result of filtering is also premultiplied. Since the resulting colors of the filtered images are the source for the renderers, one should use the premultiplied renderers, that is, agg::pixfmt_rgba32_pre, or the new one, agg::pixfmt_rgb24_pre. But it's important only if images are translucent, that is, have actual alpha channel. 

For example, if you generate some pattern with AGG (premultiplied) and would like to use it for filling, you'll need to use agg::pixfmt_rgba32_pre. If you use agg::span_image_filter_rgb24_gamma_bilinear (that is, RGB for input) and draw it on the RGBA canvas, you still need to use agg::pixfmt_rgba32_pre as the destination canvas. The only thing you need is to premultiply the background color used out of bounds.

*/


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

template void composite<mapnik::image_data_32,mapnik::image_data_32>(mapnik::image_data_32&,
                                                                     mapnik::image_data_32&,
                                                                     composite_mode_e,
                                                                     float,
                                                                     int,
                                                                     int,
                                                                     bool);

}
