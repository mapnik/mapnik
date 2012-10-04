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
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_pixfmt.hpp>
#include <mapnik/grid/grid_pixel.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/segment.hpp>
#include <mapnik/expression_evaluator.hpp>

// boost
#include <boost/scoped_ptr.hpp>

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
    typedef coord_transform<CoordTransform,geometry_type> path_type;
    typedef agg::renderer_base<mapnik::pixfmt_gray32> ren_base;
    typedef agg::renderer_scanline_bin_solid<ren_base> renderer;
    agg::scanline_bin sl;

    grid_rendering_buffer buf(pixmap_.raw_data(), width_, height_, width_);
    mapnik::pixfmt_gray32 pixf(buf);

    ren_base renb(pixf);
    renderer ren(renb);

    ras_ptr->reset();

    double height = 0.0;
    expression_ptr height_expr = sym.height();
    if (height_expr)
    {
        value_type result = boost::apply_visitor(evaluate<Feature,value_type>(feature), *height_expr);
        height = result.to_double() * scale_factor_;
    }

    for (unsigned i=0;i<feature.num_geometries();++i)
    {
        geometry_type & geom = feature.get_geometry(i);
        if (geom.size() > 2)
        {
            boost::scoped_ptr<geometry_type> frame(new geometry_type(LineString));
            boost::scoped_ptr<geometry_type> roof(new geometry_type(Polygon));
            std::deque<segment_t> face_segments;
            double x0(0);
            double y0(0);
            unsigned cm = geom.vertex(&x0,&y0);
            for (unsigned j=1;j<geom.size();++j)
            {
                double x(0);
                double y(0);
                cm = geom.vertex(&x,&y);
                if (cm == SEG_MOVETO)
                {
                    frame->move_to(x,y);
                }
                else if (cm == SEG_LINETO)
                {
                    frame->line_to(x,y);
                    face_segments.push_back(segment_t(x0,y0,x,y));
                }

                x0 = x;
                y0 = y;
            }
            std::sort(face_segments.begin(),face_segments.end(), y_order);
            std::deque<segment_t>::const_iterator itr=face_segments.begin();
            for (;itr!=face_segments.end();++itr)
            {
                boost::scoped_ptr<geometry_type> faces(new geometry_type(Polygon));
                faces->move_to(itr->get<0>(),itr->get<1>());
                faces->line_to(itr->get<2>(),itr->get<3>());
                faces->line_to(itr->get<2>(),itr->get<3>() + height);
                faces->line_to(itr->get<0>(),itr->get<1>() + height);

                path_type faces_path (t_,*faces,prj_trans);
                ras_ptr->add_path(faces_path);
                ren.color(mapnik::gray32(feature.id()));
                agg::render_scanlines(*ras_ptr, sl, ren);
                ras_ptr->reset();

                frame->move_to(itr->get<0>(),itr->get<1>());
                frame->line_to(itr->get<0>(),itr->get<1>()+height);
            }

            geom.rewind(0);
            for (unsigned j=0;j<geom.size();++j)
            {
                double x,y;
                unsigned cm = geom.vertex(&x,&y);
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
            }
            path_type path(t_,*frame,prj_trans);
            agg::conv_stroke<path_type> stroke(path);
            ras_ptr->add_path(stroke);
            ren.color(mapnik::gray32(feature.id()));
            agg::render_scanlines(*ras_ptr, sl, ren);
            ras_ptr->reset();

            path_type roof_path (t_,*roof,prj_trans);
            ras_ptr->add_path(roof_path);
            ren.color(mapnik::gray32(feature.id()));
            agg::render_scanlines(*ras_ptr, sl, ren);
        }
    }
    pixmap_.add_feature(feature);
}

template void grid_renderer<grid>::process(building_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);

}
