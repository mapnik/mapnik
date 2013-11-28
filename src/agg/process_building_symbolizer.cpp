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

    double height = get<double>(sym, keys::height,0.0) * scale_factor_;
    for (auto const& geom : feature.paths())
    {
        if (geom.size() > 2)
        {
            const std::unique_ptr<geometry_type> frame(new geometry_type(geometry_type::types::LineString));
            const std::unique_ptr<geometry_type> roof(new geometry_type(geometry_type::types::Polygon));
            std::deque<segment_t> face_segments;
            double x0 = 0;
            double y0 = 0;
            double x,y;
            geom.rewind(0);
            for (unsigned cm = geom.vertex(&x, &y); cm != SEG_END;
                 cm = geom.vertex(&x, &y))
            {
                if (cm == SEG_MOVETO)
                {
                    frame->move_to(x,y);
                }
                else if (cm == SEG_LINETO)
                {
                    frame->line_to(x,y);
                    face_segments.push_back(segment_t(x0,y0,x,y));
                }
                else if (cm == SEG_CLOSE)
                {
                    frame->close_path();
                }
                x0 = x;
                y0 = y;
            }

            std::sort(face_segments.begin(),face_segments.end(), y_order);
            for (auto const& seg : face_segments)
            {
                const std::unique_ptr<geometry_type> faces(new geometry_type(geometry_type::types::Polygon));
                faces->move_to(std::get<0>(seg),std::get<1>(seg));
                faces->line_to(std::get<2>(seg),std::get<3>(seg));
                faces->line_to(std::get<2>(seg),std::get<3>(seg) + height);
                faces->line_to(std::get<0>(seg),std::get<1>(seg) + height);

                path_type faces_path (t_,*faces,prj_trans);
                ras_ptr->add_path(faces_path);
                ren.color(agg::rgba8_pre(int(r*0.8), int(g*0.8), int(b*0.8), int(a * opacity)));
                agg::render_scanlines(*ras_ptr, sl, ren);
                ras_ptr->reset();
                //
                frame->move_to(std::get<0>(seg),std::get<1>(seg));
                frame->line_to(std::get<0>(seg),std::get<1>(seg)+height);

            }

            geom.rewind(0);
            for (unsigned cm = geom.vertex(&x, &y); cm != SEG_END;
                 cm = geom.vertex(&x, &y))
            {
                if (cm == SEG_MOVETO)
                {
                    frame->move_to(x,y+height);
                    roof->move_to(x,y+height);
                }
                else if (cm == SEG_LINETO)
                {
                    frame->line_to(x,y+height);
                    roof->line_to(x,y+height);
                }
                else if (cm == SEG_CLOSE)
                {
                    frame->close_path();
                    roof->close_path();
                }
            }

            path_type path(t_,*frame,prj_trans);
            agg::conv_stroke<path_type> stroke(path);
            stroke.width(scale_factor_);
            ras_ptr->add_path(stroke);
            ren.color(agg::rgba8_pre(int(r*0.8), int(g*0.8), int(b*0.8), int(a * opacity)));
            agg::render_scanlines(*ras_ptr, sl, ren);
            ras_ptr->reset();

            path_type roof_path (t_,*roof,prj_trans);
            ras_ptr->add_path(roof_path);
            ren.color(agg::rgba8_pre(r, g, b, int(a * opacity)));
            agg::render_scanlines(*ras_ptr, sl, ren);

        }
    }
}

template void agg_renderer<image_32>::process(building_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);

}
