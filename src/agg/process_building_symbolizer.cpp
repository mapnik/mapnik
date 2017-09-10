/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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
#include <mapnik/make_unique.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/segment.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/renderer_common/process_building_symbolizer.hpp>
#include <mapnik/transform_path_adapter.hpp>

// stl
#include <deque>
#include <memory>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_color_rgba.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_conv_stroke.h"
#pragma GCC diagnostic pop

namespace mapnik
{

template <typename T0,typename T1>
void agg_renderer<T0,T1>::process(building_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    using transform_path_type = transform_path_adapter<view_transform, vertex_adapter>;
    using ren_base = agg::renderer_base<agg::pixfmt_rgba32_pre>;
    using renderer = agg::renderer_scanline_aa_solid<ren_base>;

    agg::rendering_buffer buf(current_buffer_->bytes(),current_buffer_->width(),current_buffer_->height(), current_buffer_->row_size());
    agg::pixfmt_rgba32_pre pixf(buf);
    ren_base renb(pixf);

    value_double opacity = get<value_double,keys::fill_opacity>(sym,feature, common_.vars_);
    color const& fill = get<color, keys::fill>(sym, feature, common_.vars_);
    unsigned r=fill.red();
    unsigned g=fill.green();
    unsigned b=fill.blue();
    unsigned a=fill.alpha();
    renderer ren(renb);
    agg::scanline_u8 sl;

    ras_ptr->reset();
    double gamma = get<value_double, keys::gamma>(sym, feature, common_.vars_);
    gamma_method_enum gamma_method = get<gamma_method_enum, keys::gamma_method>(sym, feature, common_.vars_);
    if (gamma != gamma_ || gamma_method != gamma_method_)
    {
        set_gamma_method(ras_ptr, gamma, gamma_method);
        gamma_method_ = gamma_method;
        gamma_ = gamma;
    }

    double height = get<double, keys::height>(sym, feature, common_.vars_) * common_.scale_factor_;

    render_building_symbolizer(
        feature, height,
        [&,r,g,b,a,opacity](path_type const& faces)
        {
            vertex_adapter va(faces);
            transform_path_type faces_path (this->common_.t_,va,prj_trans);
            ras_ptr->add_path(faces_path);
            ren.color(agg::rgba8_pre(int(r*0.8), int(g*0.8), int(b*0.8), int(a * opacity)));
            agg::render_scanlines(*ras_ptr, sl, ren);
            this->ras_ptr->reset();
        },
        [&,r,g,b,a,opacity](path_type const& frame)
        {
            vertex_adapter va(frame);
            transform_path_type path(common_.t_,va, prj_trans);
            agg::conv_stroke<transform_path_type> stroke(path);
            stroke.width(common_.scale_factor_);
            ras_ptr->add_path(stroke);
            ren.color(agg::rgba8_pre(int(r*0.8), int(g*0.8), int(b*0.8), int(a * opacity)));
            agg::render_scanlines(*ras_ptr, sl, ren);
            ras_ptr->reset();
        },
        [&,r,g,b,a,opacity](path_type const& roof)
        {
            vertex_adapter va(roof);
            transform_path_type roof_path (common_.t_,va,prj_trans);
            ras_ptr->add_path(roof_path);
            ren.color(agg::rgba8_pre(r, g, b, int(a * opacity)));
            agg::render_scanlines(*ras_ptr, sl, ren);
        });
}

template void agg_renderer<image_rgba8>::process(building_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);
}
