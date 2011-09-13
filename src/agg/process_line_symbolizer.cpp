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
                              Feature const& feature,
                              proj_transform const& prj_trans)
{
    typedef agg::renderer_base<agg::pixfmt_rgba32_plain> ren_base;
    typedef coord_transform2<CoordTransform,geometry_type> path_type;

    stroke const& stroke_ = sym.get_stroke();
    color const& col = stroke_.get_color();
    unsigned r=col.red();
    unsigned g=col.green();
    unsigned b=col.blue();
    unsigned a=col.alpha();
    
    agg::rendering_buffer buf(pixmap_.raw_data(),width_,height_, width_ * 4);
    agg::pixfmt_rgba32_plain pixf(buf);
    
    if (sym.get_rasterizer() == RASTERIZER_FAST)
    {
        typedef agg::renderer_outline_aa<ren_base> renderer_type;
        typedef agg::rasterizer_outline_aa<renderer_type> rasterizer_type;

        agg::line_profile_aa profile;
        //agg::line_profile_aa profile(stroke_.get_width() * scale_factor_, agg::gamma_none());
        profile.width(stroke_.get_width() * scale_factor_);
        ren_base base_ren(pixf);
        renderer_type ren(base_ren, profile);
        ren.color(agg::rgba8(r, g, b, int(a*stroke_.get_opacity())));
        //ren.clip_box(0,0,width_,height_);
        rasterizer_type ras(ren);
        ras.line_join(agg::outline_miter_accurate_join);
        ras.round_cap(true);
   
        for (unsigned i=0;i<feature.num_geometries();++i)
        {
            geometry_type const& geom = feature.get_geometry(i);
            if (geom.num_points() > 1)
            {
                path_type path(t_,geom,prj_trans);
                ras.add_path(path);
            }
        }
    }
    else
    {
        typedef agg::renderer_scanline_aa_solid<ren_base> renderer;

        agg::scanline_p8 sl;
    
        ren_base renb(pixf);
        renderer ren(renb);
        ras_ptr->reset();
        ras_ptr->gamma(agg::gamma_linear(0.0, stroke_.get_gamma()));
        
        metawriter_with_properties writer = sym.get_metawriter();
        for (unsigned i=0;i<feature.num_geometries();++i)
        {
            geometry_type const& geom = feature.get_geometry(i);
            if (geom.num_points() > 1)
            {
                path_type path(t_,geom,prj_trans);
    
                if (stroke_.has_dash())
                {
                    agg::conv_dash<path_type> dash(path);
                    dash_array const& d = stroke_.get_dash_array();
                    dash_array::const_iterator itr = d.begin();
                    dash_array::const_iterator end = d.end();
                    for (;itr != end;++itr)
                    {
                        dash.add_dash(itr->first * scale_factor_, 
                                      itr->second * scale_factor_);
                    }
    
                    agg::conv_stroke<agg::conv_dash<path_type > > stroke(dash);
    
                    line_join_e join=stroke_.get_line_join();
                    if ( join == MITER_JOIN)
                        stroke.generator().line_join(agg::miter_join);
                    else if( join == MITER_REVERT_JOIN)
                        stroke.generator().line_join(agg::miter_join);
                    else if( join == ROUND_JOIN)
                        stroke.generator().line_join(agg::round_join);
                    else
                        stroke.generator().line_join(agg::bevel_join);
    
                    line_cap_e cap=stroke_.get_line_cap();
                    if (cap == BUTT_CAP)
                        stroke.generator().line_cap(agg::butt_cap);
                    else if (cap == SQUARE_CAP)
                        stroke.generator().line_cap(agg::square_cap);
                    else
                        stroke.generator().line_cap(agg::round_cap);
    
                    stroke.generator().miter_limit(4.0);
                    stroke.generator().width(stroke_.get_width() * scale_factor_);
                    ras_ptr->add_path(stroke);
    
                }
                else
                {
                    agg::conv_stroke<path_type>  stroke(path);
                    line_join_e join=stroke_.get_line_join();
                    if ( join == MITER_JOIN)
                        stroke.generator().line_join(agg::miter_join);
                    else if( join == MITER_REVERT_JOIN)
                        stroke.generator().line_join(agg::miter_join);
                    else if( join == ROUND_JOIN)
                        stroke.generator().line_join(agg::round_join);
                    else
                        stroke.generator().line_join(agg::bevel_join);
    
                    line_cap_e cap=stroke_.get_line_cap();
                    if (cap == BUTT_CAP)
                        stroke.generator().line_cap(agg::butt_cap);
                    else if (cap == SQUARE_CAP)
                        stroke.generator().line_cap(agg::square_cap);
                    else
                        stroke.generator().line_cap(agg::round_cap);
    
                    stroke.generator().miter_limit(4.0);
                    stroke.generator().width(stroke_.get_width() * scale_factor_);
                    ras_ptr->add_path(stroke);
                    if (writer.first) writer.first->add_line(path, feature, t_, writer.second);
                }
            }
        }
        ren.color(agg::rgba8(r, g, b, int(a*stroke_.get_opacity())));
        agg::render_scanlines(*ras_ptr, sl, ren);    
    }
}


template void agg_renderer<image_32>::process(line_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);

}
 
