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
#include <mapnik/std.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_renderer_base.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/segment.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/building_symbolizer.hpp>
#include <mapnik/expression.hpp>

// boost


// stl
#include <deque>

// agg
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_bin.h"
#include "agg_conv_stroke.h"

namespace mapnik
{

template <typename T>
void grid_renderer<T>::process(building_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans)
{
    typedef typename grid_renderer_base_type::pixfmt_type pixfmt_type;
    typedef typename grid_renderer_base_type::pixfmt_type::color_type color_type;
    typedef agg::renderer_scanline_bin_solid<grid_renderer_base_type> renderer_type;
    typedef coord_transform<CoordTransform,geometry_type> path_type;
    agg::scanline_bin sl;

    grid_rendering_buffer buf(pixmap_.raw_data(), width_, height_, width_);
    pixfmt_type pixf(buf);

    grid_renderer_base_type renb(pixf);
    renderer_type ren(renb);

    ras_ptr->reset();

    double height = 0.0;
    expression_ptr height_expr = sym.height();
    if (height_expr)
    {
        value_type result = boost::apply_visitor(evaluate<feature_impl,value_type>(feature), *height_expr);
        height = result.to_double() * scale_factor_;
    }

    for (std::size_t i=0;i<feature.num_geometries();++i)
    {
        geometry_type const& geom = feature.get_geometry(i);
        if (geom.size() > 2)
        {
            const auto frame = std::make_unique<geometry_type>(geometry_type::types::LineString);
            const auto roof = std::make_unique<geometry_type>(geometry_type::types::Polygon);
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
            for ( auto const& seg : face_segments)
            {
                const auto faces = std::make_unique<geometry_type>(geometry_type::types::Polygon);
                faces->move_to(std::get<0>(seg),std::get<1>(seg));
                faces->line_to(std::get<2>(seg),std::get<3>(seg));
                faces->line_to(std::get<2>(seg),std::get<3>(seg) + height);
                faces->line_to(std::get<0>(seg),std::get<1>(seg) + height);

                path_type faces_path (t_,*faces,prj_trans);
                ras_ptr->add_path(faces_path);
                ren.color(color_type(feature.id()));
                agg::render_scanlines(*ras_ptr, sl, ren);
                ras_ptr->reset();

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
            ras_ptr->add_path(stroke);
            ren.color(color_type(feature.id()));
            agg::render_scanlines(*ras_ptr, sl, ren);
            ras_ptr->reset();

            path_type roof_path (t_,*roof,prj_trans);
            ras_ptr->add_path(roof_path);
            ren.color(color_type(feature.id()));
            agg::render_scanlines(*ras_ptr, sl, ren);
        }
    }
    pixmap_.add_feature(feature);
}

template void grid_renderer<grid>::process(building_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);

}
