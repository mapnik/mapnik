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
#include <mapnik/agg_pattern_source.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/marker_cache.hpp>

// agg
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_outline.h"
#include "agg_rasterizer_outline_aa.h"
#include "agg_scanline_u.h"
//
#include "agg_renderer_scanline.h"
#include "agg_pattern_filters_rgba.h"
#include "agg_span_allocator.h"
#include "agg_span_pattern_rgba.h"
#include "agg_renderer_outline_image.h"

namespace mapnik {

template <typename T>
void  agg_renderer<T>::process(line_pattern_symbolizer const& sym,
                               Feature const& feature,
                               proj_transform const& prj_trans)
{
    typedef  coord_transform2<CoordTransform,geometry_type> path_type;
    typedef agg::line_image_pattern<agg::pattern_filter_bilinear_rgba8> pattern_type;
    typedef agg::renderer_base<agg::pixfmt_rgba32_plain> renderer_base;
    typedef agg::renderer_outline_image<renderer_base, pattern_type> renderer_type;
    typedef agg::rasterizer_outline_aa<renderer_type> rasterizer_type;

    agg::rendering_buffer buf(pixmap_.raw_data(),width_,height_, width_ * 4);
    agg::pixfmt_rgba32_plain pixf(buf);
    
    std::string filename = path_processor_type::evaluate( *sym.get_filename(), feature);

    boost::optional<marker_ptr> mark = marker_cache::instance()->find(filename,true);
    if (!mark || !(*mark)->is_bitmap()) return;

    boost::optional<image_ptr> pat = (*mark)->get_bitmap_data();

    if (!pat) return;
      
    renderer_base ren_base(pixf);
    agg::pattern_filter_bilinear_rgba8 filter;
    pattern_source source(*(*pat));
    pattern_type pattern (filter,source);
    renderer_type ren(ren_base, pattern);
    // TODO - should be sensitive to buffer size
    ren.clip_box(0,0,width_,height_);
    rasterizer_type ras(ren);
    metawriter_with_properties writer = sym.get_metawriter();
    for (unsigned i=0;i<feature.num_geometries();++i)
    {
        geometry_type const& geom = feature.get_geometry(i);
        if (geom.num_points() > 1)
        {
            path_type path(t_,geom,prj_trans);
            ras.add_path(path);
            if (writer.first) writer.first->add_line(path, feature, t_, writer.second);
        }
    }
}

template void agg_renderer<image_32>::process(line_pattern_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);

}
