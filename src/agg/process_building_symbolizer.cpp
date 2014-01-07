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
#include <mapnik/graphics.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/segment.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/renderer_common/process_building_symbolizer.hpp>

// stl
#include <deque>

// agg
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_color_rgba.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_conv_stroke.h"

namespace mapnik
{

template <typename T0,typename T1>
void agg_renderer<T0,T1>::process(building_symbolizer const& sym,
                              mapnik::feature_impl & feature,
                              proj_transform const& prj_trans)
{
    typedef coord_transform<CoordTransform,geometry_type> path_type;
    typedef agg::renderer_base<agg::pixfmt_rgba32_pre> ren_base;
    typedef agg::renderer_scanline_aa_solid<ren_base> renderer;

    agg::rendering_buffer buf(current_buffer_->raw_data(),current_buffer_->width(),current_buffer_->height(), current_buffer_->width() * 4);
    agg::pixfmt_rgba32_pre pixf(buf);
    ren_base renb(pixf);

    double opacity = get<value_double>(sym,keys::fill_opacity,feature, 1.0);
    color const& fill = get<mapnik::color>(sym, keys::fill, feature);
    unsigned r=fill.red();
    unsigned g=fill.green();
    unsigned b=fill.blue();
    unsigned a=fill.alpha();
    renderer ren(renb);
    agg::scanline_u8 sl;

    ras_ptr->reset();
    double gamma = get<value_double>(sym, keys::gamma, feature, 1.0);
    gamma_method_enum gamma_method = get<gamma_method_enum>(sym, keys::gamma_method, feature, GAMMA_POWER);
    if (gamma != gamma_ || gamma_method != gamma_method_)
    {
        set_gamma_method(ras_ptr, gamma, gamma_method);
        gamma_method_ = gamma_method;
        gamma_ = gamma;
    }

    double height = get<double>(sym, keys::height,0.0) * common_.scale_factor_;

    render_building_symbolizer(
        feature, height,
        [&](geometry_type &faces) {
            path_type faces_path (common_.t_,faces,prj_trans);
            ras_ptr->add_path(faces_path);
            ren.color(agg::rgba8_pre(int(r*0.8), int(g*0.8), int(b*0.8), int(a * opacity)));
            agg::render_scanlines(*ras_ptr, sl, ren);
            ras_ptr->reset();
        },
        [&](geometry_type &frame) {
            path_type path(common_.t_,frame,prj_trans);
            agg::conv_stroke<path_type> stroke(path);
            stroke.width(common_.scale_factor_);
            ras_ptr->add_path(stroke);
            ren.color(agg::rgba8_pre(int(r*0.8), int(g*0.8), int(b*0.8), int(a * opacity)));
            agg::render_scanlines(*ras_ptr, sl, ren);
            ras_ptr->reset();
        },
        [&](geometry_type &roof) {
            path_type roof_path (common_.t_,roof,prj_trans);
            ras_ptr->add_path(roof_path);
            ren.color(agg::rgba8_pre(r, g, b, int(a * opacity)));
            agg::render_scanlines(*ras_ptr, sl, ren);
        });
}

template void agg_renderer<image_32>::process(building_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);

}
