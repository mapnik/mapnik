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

// mapnik
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/polygon_symbolizer.hpp>

// agg
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
// for polygon_symbolizer
#include "agg_renderer_scanline.h"
#include "agg_conv_clip_polygon.h"
#include "agg_conv_smooth_poly1.h"
// stl
#include <string>

namespace mapnik {

template <typename T>
void agg_renderer<T>::process(polygon_symbolizer const& sym,
                              mapnik::feature_ptr const& feature,
                              proj_transform const& prj_trans)
{
    typedef agg::renderer_base<agg::pixfmt_rgba32_plain> ren_base;
    typedef agg::renderer_scanline_aa_solid<ren_base> renderer;

    color const& fill_ = sym.get_fill();
    agg::scanline_u8 sl;

    agg::rendering_buffer buf(pixmap_.raw_data(),width_,height_, width_ * 4);
    agg::pixfmt_rgba32_plain pixf(buf);

    ren_base renb(pixf);
    unsigned r=fill_.red();
    unsigned g=fill_.green();
    unsigned b=fill_.blue();
    unsigned a=fill_.alpha();
    //renb.clip_box(0,0,width_,height_);
    renderer ren(renb);

    ras_ptr->reset();

    set_gamma_method(sym,ras_ptr);
    
    //metawriter_with_properties writer = sym.get_metawriter();
    box2d<double> inflated_extent = query_extent_ * 1.1;
    for (unsigned i=0;i<feature->num_geometries();++i)
    {
        geometry_type & geom=feature->get_geometry(i);
        if (geom.num_points() > 2)
        {
            if (sym.smooth() > 0.0)
            {
                typedef agg::conv_clip_polygon<geometry_type> clipped_geometry_type;
                typedef coord_transform2<CoordTransform,clipped_geometry_type> path_type;
                typedef agg::conv_smooth_poly1_curve<path_type> smooth_type;
                clipped_geometry_type clipped(geom);
                clipped.clip_box(inflated_extent.minx(),inflated_extent.miny(),inflated_extent.maxx(),inflated_extent.maxy());
                path_type path(t_,clipped,prj_trans);
                smooth_type smooth(path);
                smooth.smooth_value(sym.smooth());
                ras_ptr->add_path(smooth);
            }
            else
            {
                typedef agg::conv_clip_polygon<geometry_type> clipped_geometry_type;
                typedef coord_transform2<CoordTransform,clipped_geometry_type> path_type;
                clipped_geometry_type clipped(geom);
                clipped.clip_box(query_extent_.minx(),query_extent_.miny(),query_extent_.maxx(),query_extent_.maxy());
                path_type path(t_,clipped,prj_trans);
                ras_ptr->add_path(path);
            }
            //if (writer.first) writer.first->add_polygon(path, *feature, t_, writer.second);
        }
    }
    ren.color(agg::rgba8(r, g, b, int(a * sym.get_opacity())));
    agg::render_scanlines(*ras_ptr, sl, ren);
}


template void agg_renderer<image_32>::process(polygon_symbolizer const&,
                                              mapnik::feature_ptr const&,
                                              proj_transform const&);

}

