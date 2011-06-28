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
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/marker_cache.hpp>

// agg
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
// for polygon_pattern_symbolizer
#include "agg_renderer_scanline.h"
#include "agg_span_allocator.h"
#include "agg_span_pattern_rgba.h"
#include "agg_image_accessors.h"



namespace mapnik {

template <typename T>
void agg_renderer<T>::process(polygon_pattern_symbolizer const& sym,
                              Feature const& feature,
                              proj_transform const& prj_trans)
{
    typedef coord_transform2<CoordTransform,geometry_type> path_type;
    typedef agg::renderer_base<agg::pixfmt_rgba32_plain> ren_base;
    typedef agg::wrap_mode_repeat wrap_x_type;
    typedef agg::wrap_mode_repeat wrap_y_type;
    typedef agg::pixfmt_alpha_blend_rgba<agg::blender_rgba32,
        agg::row_accessor<agg::int8u>, agg::pixel32_type> rendering_buffer;
    typedef agg::image_accessor_wrap<rendering_buffer,
        wrap_x_type,
        wrap_y_type> img_source_type;

    typedef agg::span_pattern_rgba<img_source_type> span_gen_type;

    typedef agg::renderer_scanline_aa<ren_base,
        agg::span_allocator<agg::rgba8>,
        span_gen_type> renderer_type;


    agg::rendering_buffer buf(pixmap_.raw_data(),width_,height_, width_ * 4);
    agg::pixfmt_rgba32_plain pixf(buf);
    ren_base renb(pixf);
    
    agg::scanline_u8 sl;
    ras_ptr->reset();
    ras_ptr->gamma(agg::gamma_linear(0.0, sym.get_gamma()));

    std::string filename = path_processor_type::evaluate( *sym.get_filename(), feature);
    boost::optional<mapnik::marker_ptr> marker;
    if ( !filename.empty() )
    {
        marker = marker_cache::instance()->find(filename, true);
    }
    else
    {
        std::clog << "### Warning: file not found: " << filename << "\n";
    }

    if (!marker || !(*marker)->is_bitmap()) return;
    

    boost::optional<image_ptr> pat = (*marker)->get_bitmap_data();

    if (!pat) return;
    
    unsigned w=(*pat)->width();
    unsigned h=(*pat)->height();
    agg::row_accessor<agg::int8u> pattern_rbuf((agg::int8u*)(*pat)->getBytes(),w,h,w*4);
    agg::span_allocator<agg::rgba8> sa;
    agg::pixfmt_alpha_blend_rgba<agg::blender_rgba32,
        agg::row_accessor<agg::int8u>, agg::pixel32_type> pixf_pattern(pattern_rbuf);
    img_source_type img_src(pixf_pattern);
    
    unsigned num_geometries = feature.num_geometries();

    pattern_alignment_e align = sym.get_alignment();
    unsigned offset_x=0;
    unsigned offset_y=0;
    
    if (align == LOCAL_ALIGNMENT)
    {
        double x0=0,y0=0;
        if (num_geometries>0)
        {
            path_type path(t_,feature.get_geometry(0),prj_trans);
            path.vertex(&x0,&y0);
        }
        offset_x = unsigned(width_-x0);
        offset_y = unsigned(height_-y0);    
    }
    
    span_gen_type sg(img_src, offset_x, offset_y);
    renderer_type rp(renb,sa, sg);
    metawriter_with_properties writer = sym.get_metawriter();
    for (unsigned i=0;i<num_geometries;++i)
    {
        geometry_type const& geom = feature.get_geometry(i);
        if (geom.num_points() > 2)
        {
            path_type path(t_,geom,prj_trans);
            ras_ptr->add_path(path);
            if (writer.first) writer.first->add_polygon(path, feature, t_, writer.second);
        }
    }
    agg::render_scanlines(*ras_ptr, sl, rp);
}


template void agg_renderer<image_32>::process(polygon_pattern_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);

}
 
