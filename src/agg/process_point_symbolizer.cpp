/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Artem Pavlenko
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
#include <mapnik/image_util.hpp>
#include <mapnik/image_cache.hpp>
#include <mapnik/svg/marker_cache.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_renderer.hpp>

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"

// stl
#include <string>

namespace mapnik {

struct rasterizer :  agg::rasterizer_scanline_aa<>, boost::noncopyable {};

template <typename T>
void agg_renderer<T>::process(point_symbolizer const& sym,
                              Feature const& feature,
                              proj_transform const& prj_trans)
{
    typedef coord_transform2<CoordTransform,geometry2d> path_type;
    typedef agg::pixfmt_rgba32 pixfmt;
    typedef agg::renderer_base<pixfmt> renderer_base;
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;
    
    double x;
    double y;
    double z=0;
    
    std::string filename = path_processor_type::evaluate( *sym.get_filename(), feature);
    boost::optional<mapnik::image_ptr> data;
    
    if ( filename.empty() )
    {
        // default OGC 4x4 black square
        data = boost::optional<mapnik::image_ptr>(new image_data_32(4,4));
        (*data)->set(0xff000000);
    }
    else if (is_svg(filename))
    {
        // SVG  
        boost::optional<path_ptr> marker;
        ras_ptr->reset();
        ras_ptr->gamma(agg::gamma_linear());
        agg::scanline_u8 sl;
        agg::rendering_buffer buf(pixmap_.raw_data(), width_, height_, width_ * 4);
        pixfmt pixf(buf);
        renderer_base renb(pixf);
        renderer_solid ren(renb);
        box2d<double> extent;
        
        marker = marker_cache::instance()->find(filename, true);

        if (marker && *marker)
        {
            double x1, y1, x2, y2;
            // FIXME:  Cache bounding box /////////////
            mapnik::svg::svg_converter_type svg_converter((*marker)->source(),
                                                          (*marker)->attributes());
            
            svg_converter.bounding_rect(&x1, &y1, &x2, &y2);
            /////////////////////
            mapnik::svg::svg_renderer<agg::path_storage, 
                                      agg::pod_bvector<mapnik::svg::path_attributes> > svg_renderer((*marker)->source(),
                                                                                                    (*marker)->attributes());
            
            for (unsigned i=0; i<feature.num_geometries(); ++i)
            {
                geometry2d const& geom = feature.get_geometry(i);  
                geom.label_position(&x,&y);
                prj_trans.backward(x,y,z);
                t_.forward(&x,&y);
                
                agg::trans_affine tr;
                boost::array<double,6> const& m = sym.get_transform();
                tr.load_from(&m[0]);
                
                tr *= agg::trans_affine_translation(x, y);
                
                tr.transform(&x1,&y1);
                tr.transform(&x2,&y2);
                
                extent.init(x1,y1,x2,y2);
                if (sym.get_allow_overlap() ||
                    detector_.has_placement(extent))
                {
                    
                    
                    svg_renderer.render(*ras_ptr, sl, ren, tr, renb.clip_box(), sym.get_opacity());
                    
                    detector_.insert(extent);
                }
            }   
        }
    }
    else
    {
        data = mapnik::image_cache::instance()->find(filename,true);
        if ( data )
        {
            for (unsigned i=0;i<feature.num_geometries();++i)
            {
                geometry2d const& geom = feature.get_geometry(i);
                
                geom.label_position(&x,&y);
                prj_trans.backward(x,y,z);
                t_.forward(&x,&y);
                int w = (*data)->width();
                int h = (*data)->height();
                int px=int(floor(x - 0.5 * w));
                int py=int(floor(y - 0.5 * h));
                box2d<double> label_ext (floor(x - 0.5 * w),
                                         floor(y - 0.5 * h),
                                         ceil (x + 0.5 * w),
                                         ceil (y + 0.5 * h));
                if (sym.get_allow_overlap() ||
                    detector_.has_placement(label_ext))
                {
                    pixmap_.set_rectangle_alpha2(*(*data),px,py,sym.get_opacity());
                    detector_.insert(label_ext);
                }
            }
        }
    }
}

template void agg_renderer<image_32>::process(point_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);

}
 
