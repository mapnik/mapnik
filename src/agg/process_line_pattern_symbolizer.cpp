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
#include <mapnik/debug.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/agg_pattern_source.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/line_pattern_symbolizer.hpp>
#include <mapnik/vertex_converters.hpp>
// agg
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_outline.h"
#include "agg_rasterizer_outline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_pattern_filters_rgba.h"
#include "agg_span_allocator.h"
#include "agg_span_pattern_rgba.h"
#include "agg_renderer_outline_image.h"
#include "agg_conv_clip_polyline.h"

namespace mapnik {

template <typename T>
void  agg_renderer<T>::process(line_pattern_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans)
{
    typedef agg::rgba8 color;
    typedef agg::order_rgba order;
    typedef agg::pixel32_type pixel_type;    
    typedef agg::comp_op_adaptor_rgba_pre<color, order> blender_type;
    typedef agg::pattern_filter_bilinear_rgba8 pattern_filter_type;
    typedef agg::line_image_pattern<pattern_filter_type> pattern_type;
    typedef agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixfmt_type;
    typedef agg::renderer_base<pixfmt_type> renderer_base;
    typedef agg::renderer_outline_image<renderer_base, pattern_type> renderer_type;
    typedef agg::rasterizer_outline_aa<renderer_type> rasterizer_type;
    
    std::string filename = path_processor_type::evaluate( *sym.get_filename(), feature);

    boost::optional<marker_ptr> mark = marker_cache::instance()->find(filename,true);
    if (!mark) return;

    if (!(*mark)->is_bitmap())
    {
        MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: Only images (not '" << filename << "') are supported in the line_pattern_symbolizer";

        return;
    }

    boost::optional<image_ptr> pat = (*mark)->get_bitmap_data();

    if (!pat) return;

    box2d<double> ext = query_extent_ * 1.0;
    
    agg::rendering_buffer buf(current_buffer_->raw_data(),width_,height_, width_ * 4);
    pixfmt_type pixf(buf);
    pixf.comp_op(static_cast<agg::comp_op_e>(sym.comp_op()));
    renderer_base ren_base(pixf);
    agg::pattern_filter_bilinear_rgba8 filter;

    pattern_source source(*(*pat));
    pattern_type pattern (filter,source);
    renderer_type ren(ren_base, pattern);    
    rasterizer_type ras(ren);
    
    agg::trans_affine tr;
    evaluate_transform(tr, feature, sym.get_transform());

    typedef boost::mpl::vector<clip_line_tag,transform_tag,smooth_tag> conv_types;
    vertex_converter<box2d<double>, rasterizer_type, line_pattern_symbolizer,
                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
        converter(ext,ras,sym,t_,prj_trans,tr,scale_factor_);
    
    if (sym.clip()) converter.set<clip_line_tag>(); //optional clip (default: true)
    converter.set<transform_tag>(); //always transform 
    if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter
    
    BOOST_FOREACH(geometry_type & geom, feature.paths())
    {
        if (geom.num_points() > 1)
        {
            converter.apply(geom);        
        }
    }
}

template void agg_renderer<image_32>::process(line_pattern_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);

}
