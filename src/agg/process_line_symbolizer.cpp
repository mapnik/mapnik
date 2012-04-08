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

// boost
#include <boost/foreach.hpp>
// mapnik
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/line_symbolizer.hpp>
#include <mapnik/vertex_converters.hpp>
// agg
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_p.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
#include "agg_renderer_outline_aa.h"
#include "agg_rasterizer_outline_aa.h"
// stl
#include <string>

namespace mapnik {


template <typename T>
void agg_renderer<T>::process(line_symbolizer const& sym,
                              mapnik::feature_ptr const& feature,
                              proj_transform const& prj_trans)

{
    stroke const& stroke_ = sym.get_stroke();
    color const& col = stroke_.get_color();
    unsigned r=col.red();
    unsigned g=col.green();
    unsigned b=col.blue();
    unsigned a=col.alpha();

    
    agg::rendering_buffer buf(current_buffer_->raw_data(),width_,height_, width_ * 4);
    //agg::pixfmt_rgba32 pixf(buf);
    aa_renderer::pixel_format_type pixf(buf);
    box2d<double> ext = query_extent_ * 1.1;
    
    if (sym.get_rasterizer() == RASTERIZER_FAST)
    {
        /*
        typedef agg::renderer_base<aa_renderer::pixel_format_type> ren_base;
        typedef agg::renderer_outline_aa<ren_base> renderer_type;
        typedef agg::rasterizer_outline_aa<renderer_type> rasterizer_type;

        //typedef agg::conv_clip_polyline<geometry_type> clipped_geometry_type;
        //typedef coord_transform2<CoordTransform,clipped_geometry_type> path_type;

        agg::line_profile_aa profile;
        profile.width(stroke_.get_width() * scale_factor_);
        ren_base base_ren(pixf);
        renderer_type ren(base_ren, profile);
        ren.color(agg::rgba8(r, g, b, int(a*stroke_.get_opacity())));
        //ren.clip_box(0,0,width_,height_);
        rasterizer_type ras(ren);
        ras.line_join(agg::outline_miter_accurate_join);
        ras.round_cap(true);
        
        for (unsigned i=0;i<feature->num_geometries();++i)
        {
            geometry_type & geom = feature->get_geometry(i);
            if (geom.num_points() > 1)
            {
                clipped_geometry_type clipped(geom);
                clipped.clip_box(ext.minx(),ext.miny(),ext.maxx(),ext.maxy());
                path_type path(t_,clipped,prj_trans);
                ras.add_path(path);
            }
        }
        */
    }
    else
    {        
        ras_ptr->reset();        
        set_gamma_method(stroke_, ras_ptr);
        aa_renderer ren;
        ren.attach(pixf);
        
        //metawriter_with_properties writer = sym.get_metawriter();
        typedef boost::mpl::vector<clip_line_tag,transform_tag, smooth_tag, dash_tag, stroke_tag> conv_types;
        vertex_converter<box2d<double>,rasterizer,line_symbolizer, proj_transform, CoordTransform,conv_types> 
            converter(ext,*ras_ptr,sym,t_,prj_trans);
        
        //if (sym.clip()) 
        converter.set<clip_line_tag>(); //FIXME make an optinal clip (default: true) 
        converter.set<transform_tag>(); //always transform 
        if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter
        if (stroke_.has_dash()) converter.set<dash_tag>();        
        converter.set<stroke_tag>(); //always stroke
        
        BOOST_FOREACH( geometry_type & geom, feature->paths())
        {
            if (geom.num_points() > 1)
            {
                converter.apply(geom);                
            }
        }
        
        agg::rendering_buffer buf(current_buffer_->raw_data(),width_,height_, width_ * 4);

        if (sym.comp_op() == clear)
        {
            aa_renderer::pixfmt_type pixf(buf);
            aa_renderer ren;
            ren.attach(pixf);
            ren.color(agg::rgba8(r, g, b, int(a * stroke_.get_opacity())));
            ren.render(*ras_ptr);
        }
        else
        {        
            typedef agg::rgba8 color_type;
            typedef agg::order_rgba order_type;
            typedef agg::pixel32_type pixel_type;
            typedef agg::comp_op_adaptor_rgba<color_type, order_type> blender_type; // comp blender
            typedef agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixfmt_comp_type;
            typedef agg::renderer_base<pixfmt_comp_type> renderer_base;
            typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_type;     
            pixfmt_comp_type pixf(buf);
            pixf.comp_op(static_cast<agg::comp_op_e>(sym.comp_op()));
            renderer_base renb(pixf);
            renderer_type ren(renb);
            ren.color(agg::rgba8(r, g, b, int(a * stroke_.get_opacity())));
            agg::scanline_u8 sl;
            agg::render_scanlines(*ras_ptr, sl, ren);
        }
    }
}


template void agg_renderer<image_32>::process(line_symbolizer const&,
                                              mapnik::feature_ptr const&,
                                              proj_transform const&);

}

