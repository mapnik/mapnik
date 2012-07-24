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
#include <mapnik/line_pattern_symbolizer.hpp>

// agg
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_bin.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"

// stl
#include <string>

namespace mapnik {

template <typename T>
void grid_renderer<T>::process(line_pattern_symbolizer const& sym,
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

    // TODO - actually handle image dimensions
    int stroke_width = 2;

    for (unsigned i=0;i<feature.num_geometries();++i)
    {
        geometry_type & geom = feature.get_geometry(i);
        if (geom.size() > 1)
        {
            path_type path(t_,geom,prj_trans);
            agg::conv_stroke<path_type> stroke(path);
            stroke.generator().miter_limit(4.0);
            stroke.generator().width(stroke_width * scale_factor_);
            ras_ptr->add_path(stroke);
        }
    }

    // render id
    ren.color(mapnik::gray32(feature.id()));
    agg::render_scanlines(*ras_ptr, sl, ren);

    // add feature properties to grid cache
    pixmap_.add_feature(feature);

}


template void grid_renderer<grid>::process(line_pattern_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);

}
