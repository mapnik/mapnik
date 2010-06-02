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

#include <mapnik/agg_renderer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_cache.hpp>
#include <mapnik/svg/marker_cache.hpp>
#include <mapnik/markers_placement.hpp>
#include <mapnik/arrow.hpp>

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"

namespace mapnik {

struct rasterizer :  agg::rasterizer_scanline_aa<>, boost::noncopyable {};

template <typename T>
void agg_renderer<T>::process(markers_symbolizer const& sym,
                              Feature const& feature,
                              proj_transform const& prj_trans)
{
    typedef coord_transform2<CoordTransform,geometry2d> path_type;
    typedef agg::pixfmt_rgba32 pixfmt;
    typedef agg::renderer_base<pixfmt> renderer_base;
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;

    bool svg_marker;
    arrow arrow_;
    box2d<double> extent;
    boost::optional<path_ptr> marker;

    ras_ptr->reset();
    ras_ptr->gamma(agg::gamma_linear());
    agg::scanline_u8 sl;
    agg::rendering_buffer buf(pixmap_.raw_data(), width_, height_, width_ * 4);
    pixfmt pixf(buf);
    renderer_base renb(pixf);
    renderer_solid ren(renb);

    color const& fill_ = sym.get_fill();
    unsigned r = fill_.red();
    unsigned g = fill_.green();
    unsigned b = fill_.blue();
    unsigned a = fill_.alpha();
    
    svg_marker = false;
    extent = arrow_.extent();        
    
    std::string filename = path_processor_type::evaluate(*sym.get_filename(), feature);
    
    if (!filename.empty())
    {
        marker = mapnik::marker_cache::instance()->find(filename, true);
        if (marker && *marker)
        {
            svg_marker = true;
            double x1, y1, x2, y2;
            (*marker)->bounding_rect(&x1, &y1, &x2, &y2);
            extent.init(x1, y1, x2, y2);
        }
    }
    
    for (unsigned i=0; i<feature.num_geometries(); ++i)
    {
        geometry2d const& geom = feature.get_geometry(i);
        if (geom.num_points() <= 1) continue;

        path_type path(t_,geom,prj_trans);
        markers_placement<path_type, label_collision_detector4> placement(path, extent, detector_, 
                                                                          sym.get_spacing(), 
                                                                          sym.get_max_error(), 
                                                                          sym.get_allow_overlap());        
        double x, y, angle;
        
        while (placement.get_point(&x, &y, &angle))
        {
            agg::trans_affine matrix = agg::trans_affine_rotation(angle) * agg::trans_affine_translation(x, y);
            if (svg_marker)
            {
                (*marker)->render(*ras_ptr, sl, ren, matrix, renb.clip_box(), 1.0);
            }
            else
            {
                agg::conv_transform<arrow, agg::trans_affine> trans(arrow_, matrix);
                ras_ptr->add_path(trans);
                ren.color(agg::rgba8(r, g, b, a));
                agg::render_scanlines(*ras_ptr, sl, ren);
            }
        }
    }
}

template void agg_renderer<image_32>::process(markers_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);
}
