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
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_pixfmt.hpp>
#include <mapnik/grid/grid_pixel.hpp>
#include <mapnik/grid/grid.hpp>

// agg
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_p.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"

// stl
#include <string>

namespace mapnik {

template <typename T>
void grid_renderer<T>::process(line_symbolizer const& sym,
                              Feature const& feature,
                              proj_transform const& prj_trans)
{
    typedef coord_transform2<CoordTransform,geometry_type> path_type;
    typedef agg::renderer_base<mapnik::pixfmt_gray16> ren_base;
    typedef agg::renderer_scanline_aa_solid<ren_base> renderer;
    //agg::scanline_u8 sl;
    agg::scanline_p8 sl;

    grid_rendering_buffer buf(pixmap_.raw_data(), width_, height_, width_);
    mapnik::pixfmt_gray16 pixf(buf);

    ren_base renb(pixf);
    renderer ren(renb);

    ras_ptr->reset();
    ras_ptr->gamma(agg::gamma_linear(0.0, 0.0));

    stroke const&  stroke_ = sym.get_stroke();

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
            }
        }
    }

    // render id
    ren.color(mapnik::gray16(feature.id()));
    agg::render_scanlines(*ras_ptr, sl, ren);

    // add feature properties to grid cache
    pixmap_.add_feature(feature);

}


template void grid_renderer<grid>::process(line_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);

}
 
