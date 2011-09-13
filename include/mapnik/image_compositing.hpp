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

//$Id$

#ifndef IMAGE_COMPOSITING_HPP
#define IMAGE_COMPOSITING_HPP

// agg
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_pixfmt_rgba.h"

namespace mapnik
{

// Compositing modes 
// http://www.w3.org/TR/2009/WD-SVGCompositing-20090430/

enum composite_mode_e
{
    clear = 1,
    src,
    dst,
    src_over,
    dst_over,
    src_in,
    dst_in,
    src_out,
    dst_out,
    src_atop,
    dst_atop,
    _xor,
    plus,
    minus,
    multiply,
    screen,
    overlay,
    darken,
    lighten,
    color_dodge,
    color_burn,
    hard_light,
    soft_light,
    difference,
    exclusion,
    contrast,
    invert,
    invert_rgb
};

template <typename T1, typename T2>
void composite(T1 & im, T2 & im2, composite_mode_e mode)
{
    typedef agg::rgba8 color;
    typedef agg::order_bgra order;
    typedef agg::pixel32_type pixel_type;
    typedef agg::comp_op_adaptor_rgba<color, order> blender_type;
    typedef agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixfmt_type;
    typedef agg::renderer_base<pixfmt_type> renderer_type;
    typedef agg::comp_op_adaptor_rgba<color, order> blender_type;
    typedef agg::renderer_base<pixfmt_type> renderer_type;
    
    agg::rendering_buffer source(im.getBytes(),im.width(),im.height(),im.width() * 4);
    agg::rendering_buffer mask(im2.getBytes(),im2.width(),im2.height(),im2.width() * 4);
    
    agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixf(source);
    agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixf_mask(mask);
    
    switch(mode)
    {
    case clear :
        pixf.comp_op(agg::comp_op_clear);
        break;
    case src:
        pixf.comp_op(agg::comp_op_src);
        break;
    case dst:
        pixf.comp_op(agg::comp_op_dst);
        break;
    case src_over:
        pixf.comp_op(agg::comp_op_src_over);
        break;
    case dst_over:
        pixf.comp_op(agg::comp_op_dst_over);
        break;
    case src_in:
        pixf.comp_op(agg::comp_op_src_in);
        break;
    case dst_in:
        pixf.comp_op(agg::comp_op_dst_in);
        break;
    case src_out:
        pixf.comp_op(agg::comp_op_src_out);
        break;
    case dst_out:
        pixf.comp_op(agg::comp_op_dst_out);
        break;
    case src_atop:
        pixf.comp_op(agg::comp_op_src_atop);
        break;
    case dst_atop:
        pixf.comp_op(agg::comp_op_dst_atop);
        break;
    case _xor:
        pixf.comp_op(agg::comp_op_xor);
        break;
    case plus:
        pixf.comp_op(agg::comp_op_plus);
        break;
    case minus:
        pixf.comp_op(agg::comp_op_minus);
        break;
    case multiply:
        pixf.comp_op(agg::comp_op_multiply);
        break;     
    case screen:
        pixf.comp_op(agg::comp_op_screen);
        break;
    case overlay:
        pixf.comp_op(agg::comp_op_overlay);
        break;
    case darken:
        pixf.comp_op(agg::comp_op_darken);
        break;    
    case lighten:
        pixf.comp_op(agg::comp_op_lighten);
        break;
    case color_dodge:
        pixf.comp_op(agg::comp_op_color_dodge);
        break;
    case color_burn:
        pixf.comp_op(agg::comp_op_color_burn);
        break;
    case hard_light:
        pixf.comp_op(agg::comp_op_hard_light);
        break;
    case soft_light:
        pixf.comp_op(agg::comp_op_soft_light);
        break;
    case difference:
        pixf.comp_op(agg::comp_op_difference);
        break;
    case exclusion:
        pixf.comp_op(agg::comp_op_exclusion);
        break;
    case contrast:
        pixf.comp_op(agg::comp_op_contrast);
        break;
    case invert:
        pixf.comp_op(agg::comp_op_invert);
        break;
    case invert_rgb:
        pixf.comp_op(agg::comp_op_invert_rgb);
        break;
    default:
        break;
    
    }
    renderer_type ren(pixf);
    agg::renderer_base<pixfmt_type> rb(pixf);
    rb.blend_from(pixf_mask,0,0,0,255);

}

}

#endif // IMAGE_COMPOSITING_HPP
