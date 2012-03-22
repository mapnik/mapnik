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
#include <mapnik/line_symbolizer.hpp>

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
#include "agg_conv_clip_polyline.h"
#include "agg_conv_smooth_poly1.h"
// stl
#include <string>

namespace mapnik {

template <typename T>
void agg_renderer<T>::process(line_symbolizer const& sym,
                              mapnik::feature_ptr const& feature,
                              proj_transform const& prj_trans)
{
    typedef agg::renderer_base<agg::pixfmt_rgba32_plain> ren_base;

    stroke const& stroke_ = sym.get_stroke();
    color const& col = stroke_.get_color();
    unsigned r=col.red();
    unsigned g=col.green();
    unsigned b=col.blue();
    unsigned a=col.alpha();

    agg::rendering_buffer buf(pixmap_.raw_data(),width_,height_, width_ * 4);
    agg::pixfmt_rgba32_plain pixf(buf);

    box2d<double> ext = query_extent_ * 1.1;
    if (sym.get_rasterizer() == RASTERIZER_FAST)
    {
        typedef agg::renderer_outline_aa<ren_base> renderer_type;
        typedef agg::rasterizer_outline_aa<renderer_type> rasterizer_type;
        typedef agg::conv_clip_polyline<geometry_type> clipped_geometry_type;
        typedef coord_transform2<CoordTransform,clipped_geometry_type> path_type;

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
    }
    else
    {
        typedef agg::renderer_scanline_aa_solid<ren_base> renderer;

        agg::scanline_p8 sl;

        ren_base renb(pixf);
        renderer ren(renb);
        ras_ptr->reset();

        set_gamma_method(stroke_, ras_ptr);

        //metawriter_with_properties writer = sym.get_metawriter();
        for (unsigned i=0;i<feature->num_geometries();++i)
        {
            geometry_type & geom = feature->get_geometry(i);
            if (geom.num_points() > 1)
            {
                if (stroke_.has_dash())
                {
                    if (sym.smooth() > 0.0)
                    {
                        typedef agg::conv_clip_polyline<geometry_type> clipped_geometry_type;
                        typedef coord_transform2<CoordTransform,clipped_geometry_type> path_type;
                        typedef agg::conv_smooth_poly1_curve<path_type> smooth_type;
                        clipped_geometry_type clipped(geom);
                        clipped.clip_box(ext.minx(),ext.miny(),ext.maxx(),ext.maxy());
                        path_type path(t_,clipped,prj_trans);
                        smooth_type smooth(path);
                        smooth.smooth_value(sym.smooth());
                        agg::conv_dash<smooth_type> dash(smooth);
                        dash_array const& d = stroke_.get_dash_array();
                        dash_array::const_iterator itr = d.begin();
                        dash_array::const_iterator end = d.end();
                        for (;itr != end;++itr)
                        {
                            dash.add_dash(itr->first * scale_factor_,
                                          itr->second * scale_factor_);
                        }
                        agg::conv_stroke<agg::conv_dash<smooth_type > > stroke(dash);
                        set_join_caps(stroke_,stroke);
                        stroke.generator().miter_limit(4.0);
                        stroke.generator().width(stroke_.get_width() * scale_factor_);
                        ras_ptr->add_path(stroke);
                    }
                    else
                    {
                        typedef agg::conv_clip_polyline<geometry_type> clipped_geometry_type;
                        typedef coord_transform2<CoordTransform,clipped_geometry_type> path_type;
                        clipped_geometry_type clipped(geom);
                        clipped.clip_box(ext.minx(),ext.miny(),ext.maxx(),ext.maxy());
                        path_type path(t_,clipped,prj_trans);

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
                        set_join_caps(stroke_,stroke);
                        stroke.generator().miter_limit(4.0);
                        stroke.generator().width(stroke_.get_width() * scale_factor_);
                        ras_ptr->add_path(stroke);
                    }
                }
                else
                {
                    if (sym.smooth() > 0.0)
                    {
                        typedef agg::conv_clip_polyline<geometry_type> clipped_geometry_type;
                        typedef coord_transform2<CoordTransform,clipped_geometry_type> path_type;
                        typedef agg::conv_smooth_poly1_curve<path_type> smooth_type;
                        clipped_geometry_type clipped(geom);
                        clipped.clip_box(ext.minx(),ext.miny(),ext.maxx(),ext.maxy());
                        path_type path(t_,clipped,prj_trans);
                        smooth_type smooth(path);
                        smooth.smooth_value(sym.smooth());
                        agg::conv_stroke<smooth_type> stroke(smooth);
                        set_join_caps(stroke_,stroke);
                        stroke.generator().miter_limit(4.0);
                        stroke.generator().width(stroke_.get_width() * scale_factor_);
                        ras_ptr->add_path(stroke);
                    }
                    else
                    {
                        typedef agg::conv_clip_polyline<geometry_type> clipped_geometry_type;
                        typedef coord_transform2<CoordTransform,clipped_geometry_type> path_type;
                        clipped_geometry_type clipped(geom);
                        clipped.clip_box(ext.minx(),ext.miny(),ext.maxx(),ext.maxy());
                        path_type path(t_,clipped,prj_trans);
                        agg::conv_stroke<path_type> stroke(path);
                        set_join_caps(stroke_,stroke);
                        stroke.generator().miter_limit(4.0);
                        stroke.generator().width(stroke_.get_width() * scale_factor_);
                        ras_ptr->add_path(stroke);
                    }
                    //if (writer.first) writer.first->add_line(path, *feature, t_, writer.second);
                }
            }
        }
        ren.color(agg::rgba8(r, g, b, int(a*stroke_.get_opacity())));
        agg::render_scanlines(*ras_ptr, sl, ren);
    }
}


template void agg_renderer<image_32>::process(line_symbolizer const&,
                                              mapnik::feature_ptr const&,
                                              proj_transform const&);

}

