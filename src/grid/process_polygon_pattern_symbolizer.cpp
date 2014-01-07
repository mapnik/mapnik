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


// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_renderer_base.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/parse_path.hpp>

// agg
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_bin.h"

// stl
#include <string>
#include <map>

namespace mapnik {

template <typename T>
void grid_renderer<T>::process(polygon_pattern_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans)
{
    std::string filename = get<std::string>(sym, keys::file, feature);

    boost::optional<marker_ptr> mark = marker_cache::instance().find(filename,true);
    if (!mark) return;

    if (!(*mark)->is_bitmap())
    {
        MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: Only images (not '" << filename << "') are supported in the line_pattern_symbolizer";
        return;
    }

    boost::optional<image_ptr> pat = (*mark)->get_bitmap_data();
    if (!pat) return;

    ras_ptr->reset();

    bool clip = get<value_bool>(sym, keys::clip, feature, false);
    double simplify_tolerance = get<value_double>(sym, keys::simplify_tolerance, feature, 0.0);
    double smooth = get<value_double>(sym, keys::smooth, feature, false);

    agg::trans_affine tr;
    auto geom_transform = get_optional<transform_type>(sym, keys::geometry_transform);
    if (geom_transform) evaluate_transform(tr, feature, *geom_transform);

    typedef boost::mpl::vector<clip_poly_tag,transform_tag,affine_transform_tag,smooth_tag> conv_types;
    vertex_converter<box2d<double>, grid_rasterizer, polygon_pattern_symbolizer,
                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
        converter(common_.query_extent_,*ras_ptr,sym,common_.t_,prj_trans,tr,common_.scale_factor_);

    if (prj_trans.equal() && clip) converter.set<clip_poly_tag>(); //optional clip (default: true)
    converter.set<transform_tag>(); //always transform
    converter.set<affine_transform_tag>();
    if (simplify_tolerance > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (smooth > 0.0) converter.set<smooth_tag>(); // optional smooth converter

    for ( geometry_type & geom : feature.paths())
    {
        if (geom.size() > 2)
        {
            converter.apply(geom);
        }
    }
    typedef typename grid_renderer_base_type::pixfmt_type pixfmt_type;
    typedef typename grid_renderer_base_type::pixfmt_type::color_type color_type;
    typedef agg::renderer_scanline_bin_solid<grid_renderer_base_type> renderer_type;

    grid_rendering_buffer buf(pixmap_.raw_data(), common_.width_, common_.height_, common_.width_);
    pixfmt_type pixf(buf);

    grid_renderer_base_type renb(pixf);
    renderer_type ren(renb);

    // render id
    ren.color(color_type(feature.id()));
    agg::scanline_bin sl;
    ras_ptr->filling_rule(agg::fill_even_odd);
    agg::render_scanlines(*ras_ptr, sl, ren);

    // add feature properties to grid cache
    pixmap_.add_feature(feature);
}


template void grid_renderer<grid>::process(polygon_pattern_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);

}
