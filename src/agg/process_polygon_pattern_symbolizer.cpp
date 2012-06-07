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
#include <mapnik/agg_helpers.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/vertex_converters.hpp>
// agg
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
// for polygon_pattern_symbolizer
#include "agg_renderer_scanline.h"
#include "agg_span_allocator.h"
#include "agg_span_pattern_rgba.h"
#include "agg_image_accessors.h"
#include "agg_conv_clip_polygon.h"

namespace mapnik {

template <typename T>
void agg_renderer<T>::process(polygon_pattern_symbolizer const& sym,
                              mapnik::feature_ptr const& feature,
                              proj_transform const& prj_trans)
{
    typedef agg::conv_clip_polygon<geometry_type> clipped_geometry_type;
    typedef coord_transform<CoordTransform,clipped_geometry_type> path_type;

    agg::rendering_buffer buf(current_buffer_->raw_data(), width_, height_, width_ * 4);
    ras_ptr->reset();
    set_gamma_method(sym,ras_ptr);

    std::string filename = path_processor_type::evaluate( *sym.get_filename(), *feature);
    boost::optional<mapnik::marker_ptr> marker;
    if ( !filename.empty() )
    {
        marker = marker_cache::instance()->find(filename, true);
    }
    else
    {
        MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: File not found=" << filename;
    }

    if (!marker) return;

    if (!(*marker)->is_bitmap())
    {
        MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: Only images (not '" << filename << "') are supported in the polygon_pattern_symbolizer";

        return;
    }

    boost::optional<image_ptr> pat = (*marker)->get_bitmap_data();

    if (!pat) return;

    typedef agg::rgba8 color;
    typedef agg::order_rgba order;
    typedef agg::pixel32_type pixel_type;
    typedef agg::comp_op_adaptor_rgba<color, order> blender_type;
    typedef agg::wrap_mode_repeat wrap_x_type;
    typedef agg::wrap_mode_repeat wrap_y_type;
    typedef agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixfmt_type;
    typedef agg::image_accessor_wrap<agg::pixfmt_rgba32,
                                     wrap_x_type,
                                     wrap_y_type> img_source_type;
    typedef agg::span_pattern_rgba<img_source_type> span_gen_type;
    typedef agg::renderer_base<pixfmt_type> ren_base;

    typedef agg::renderer_scanline_aa<ren_base,
        agg::span_allocator<agg::rgba8>,
        span_gen_type> renderer_type;

    pixfmt_type pixf(buf);
    pixf.comp_op(static_cast<agg::comp_op_e>(sym.comp_op()));
    ren_base renb(pixf);

    unsigned w=(*pat)->width();
    unsigned h=(*pat)->height();
    agg::rendering_buffer pattern_rbuf((agg::int8u*)(*pat)->getBytes(),w,h,w*4);
    agg::pixfmt_rgba32 pixf_pattern(pattern_rbuf);
    img_source_type img_src(pixf_pattern);



    pattern_alignment_e align = sym.get_alignment();
    unsigned offset_x=0;
    unsigned offset_y=0;

    if (align == LOCAL_ALIGNMENT)
    {
        double x0 = 0;
        double y0 = 0;
        if (feature->num_geometries() > 0)
        {
            clipped_geometry_type clipped(feature->get_geometry(0));
            clipped.clip_box(query_extent_.minx(),query_extent_.miny(),query_extent_.maxx(),query_extent_.maxy());
            path_type path(t_,clipped,prj_trans);
            path.vertex(&x0,&y0);
        }
        offset_x = unsigned(width_ - x0);
        offset_y = unsigned(height_ - y0);
    }

    span_gen_type sg(img_src, offset_x, offset_y);

    agg::span_allocator<agg::rgba8> sa;
    renderer_type rp(renb,sa, sg);

    agg::trans_affine tr;
    evaluate_transform(tr, *feature, sym.get_transform());

    box2d<double> inflated_extent = query_extent_ * 1.0;
    typedef boost::mpl::vector<clip_poly_tag,transform_tag,smooth_tag> conv_types;
    vertex_converter<box2d<double>, rasterizer, polygon_pattern_symbolizer,
                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
        converter(inflated_extent,*ras_ptr,sym,t_,prj_trans,tr,scale_factor_);

    if (sym.clip()) converter.set<clip_poly_tag>(); //optional clip (default: true)
    converter.set<transform_tag>(); //always transform
    if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter

    BOOST_FOREACH( geometry_type & geom, feature->paths())
    {
        if (geom.num_points() > 2)
        {
            converter.apply(geom);
        }
    }
    agg::scanline_u8 sl;
    agg::render_scanlines(*ras_ptr, sl, rp);
}


template void agg_renderer<image_32>::process(polygon_pattern_symbolizer const&,
                                              mapnik::feature_ptr const&,
                                              proj_transform const&);

}
