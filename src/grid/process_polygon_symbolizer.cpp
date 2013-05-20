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

// boost
#include <boost/foreach.hpp>

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_renderer_base.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/polygon_symbolizer.hpp>
#include <mapnik/vertex_converters.hpp>

// agg
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_bin.h"

// stl
#include <string>
#include <map>

namespace mapnik {

template <typename T>
void grid_renderer<T>::process(polygon_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans)
{
    typedef agg::renderer_scanline_bin_solid<grid_renderer_base_type> renderer_type;
    typedef typename grid_renderer_base_type::pixfmt_type pixfmt_type;
    typedef typename grid_renderer_base_type::pixfmt_type::color_type color_type;

    ras_ptr->reset();

    agg::trans_affine tr;
    evaluate_transform(tr, feature, sym.get_transform());

    typedef boost::mpl::vector<clip_poly_tag,transform_tag,affine_transform_tag,simplify_tag,smooth_tag> conv_types;
    vertex_converter<box2d<double>, grid_rasterizer, polygon_symbolizer,
                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
        converter(query_extent_,*ras_ptr,sym,t_,prj_trans,tr,scale_factor_);

    if (prj_trans.equal() && sym.clip()) converter.set<clip_poly_tag>(); //optional clip (default: true)
    converter.set<transform_tag>(); //always transform
    converter.set<affine_transform_tag>();
    if (sym.simplify_tolerance() > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter


    BOOST_FOREACH( geometry_type & geom, feature.paths())
    {
        if (geom.size() > 2)
        {
            converter.apply(geom);
        }
    }

    grid_rendering_buffer buf(pixmap_.raw_data(), width_, height_, width_);
    pixfmt_type pixf(buf);

    grid_renderer_base_type renb(pixf);
    renderer_type ren(renb);

    // render id
    ren.color(color_type(feature.id()));
    agg::scanline_bin sl;
    agg::render_scanlines(*ras_ptr, sl, ren);

    // add feature properties to grid cache
    pixmap_.add_feature(feature);
}


template void grid_renderer<grid>::process(polygon_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);

}

