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
#include <mapnik/vertex_converters.hpp>

// agg
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"

namespace mapnik {

template <typename T>
void agg_renderer<T>::process(polygon_symbolizer const& sym,
                              mapnik::feature_ptr const& feature,
                              proj_transform const& prj_trans)
{
    agg::rendering_buffer buf(current_buffer_->raw_data(),width_,height_, width_ * 4);
    aa_renderer::pixel_format_type pixf(buf);
    
    ras_ptr->reset();
    set_gamma_method(sym,ras_ptr);
    aa_renderer ren;
    ren.attach(pixf);
    
    box2d<double> inflated_extent = query_extent_ * 1.1;
    
    typedef boost::mpl::vector<smooth> conv_types;
    vertex_converter<box2d<double>, rasterizer,polygon_symbolizer,conv_types> converter(inflated_extent,*ras_ptr,sym);
    if (sym.smooth() > 0.0) converter.set<smooth>();    
    //converter.set<clip_poly>(); //always clip 
    
    for (unsigned i=0;i<feature->num_geometries();++i)
    {
        geometry_type & geom=feature->get_geometry(i);
        if (geom.num_points() > 2)
        {
            typedef agg::conv_clip_polygon<geometry_type> clipped_geometry_type;
            typedef coord_transform2<CoordTransform,clipped_geometry_type> path_type;
            
            
            clipped_geometry_type clipped(geom);
            clipped.clip_box(inflated_extent.minx(),inflated_extent.miny(),inflated_extent.maxx(),inflated_extent.maxy());
            path_type path(t_,clipped,prj_trans);
            
            converter.apply(path);
            
        }
    }
    
    color const& fill_ = sym.get_fill();
    unsigned r=fill_.red();
    unsigned g=fill_.green();
    unsigned b=fill_.blue();
    unsigned a=fill_.alpha();
    ren.color(agg::rgba8(r, g, b, int(a * sym.get_opacity())));
    ren.render(*ras_ptr);
    
}

template void agg_renderer<image_32>::process(polygon_symbolizer const&,
                                              mapnik::feature_ptr const&,
                                              proj_transform const&);

}

