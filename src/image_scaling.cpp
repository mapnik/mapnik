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
#include <mapnik/image_data.hpp>
#include <mapnik/image_scaling.hpp>
// does not handle alpha correctly
//#include <mapnik/span_image_filter.hpp>

// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wredeclared-class-member"
#include <boost/assign/list_of.hpp>
#include <boost/bimap.hpp>
#pragma GCC diagnostic pop

// agg
#include "agg_image_accessors.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_rendering_buffer.h"
#include "agg_scanline_u.h"
#include "agg_span_allocator.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_span_interpolator_linear.h"
#include "agg_trans_affine.h"
#include "agg_image_filters.h"

namespace mapnik
{

using scaling_method_lookup_type = boost::bimap<scaling_method_e, std::string>;
static const scaling_method_lookup_type scaling_lookup = boost::assign::list_of<scaling_method_lookup_type::relation>
    (SCALING_NEAR,"near")
    (SCALING_BILINEAR,"bilinear")
    (SCALING_BICUBIC,"bicubic")
    (SCALING_SPLINE16,"spline16")
    (SCALING_SPLINE36,"spline36")
    (SCALING_HANNING,"hanning")
    (SCALING_HAMMING,"hamming")
    (SCALING_HERMITE,"hermite")
    (SCALING_KAISER,"kaiser")
    (SCALING_QUADRIC,"quadric")
    (SCALING_CATROM,"catrom")
    (SCALING_GAUSSIAN,"gaussian")
    (SCALING_BESSEL,"bessel")
    (SCALING_MITCHELL,"mitchell")
    (SCALING_SINC,"sinc")
    (SCALING_LANCZOS,"lanczos")
    (SCALING_BLACKMAN,"blackman")
    ;

boost::optional<scaling_method_e> scaling_method_from_string(std::string const& name)
{
    boost::optional<scaling_method_e> mode;
    scaling_method_lookup_type::right_const_iterator right_iter = scaling_lookup.right.find(name);
    if (right_iter != scaling_lookup.right.end())
    {
        mode.reset(right_iter->second);
    }
    return mode;
}

boost::optional<std::string> scaling_method_to_string(scaling_method_e scaling_method)
{
    boost::optional<std::string> mode;
    scaling_method_lookup_type::left_const_iterator left_iter = scaling_lookup.left.find(scaling_method);
    if (left_iter != scaling_lookup.left.end())
    {
        mode.reset(left_iter->second);
    }
    return mode;
}

template <typename Image>
void scale_image_agg(Image & target,
                     Image const& source,
                     scaling_method_e scaling_method,
                     double image_ratio_x,
                     double image_ratio_y,
                     double x_off_f,
                     double y_off_f,
                     double filter_factor)
{
    // "the image filters should work namely in the premultiplied color space"
    // http://old.nabble.com/Re:--AGG--Basic-image-transformations-p1110665.html
    // "Yes, you need to use premultiplied images only. Only in this case the simple weighted averaging works correctly in the image fitering."
    // http://permalink.gmane.org/gmane.comp.graphics.agg/3443
    using pixfmt_pre = agg::pixfmt_rgba32_pre;
    using renderer_base_pre = agg::renderer_base<pixfmt_pre>;

    // define some stuff we'll use soon
    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_u8 sl;
    agg::span_allocator<agg::rgba8> sa;
    agg::image_filter_lut filter;

    // initialize source AGG buffer
    agg::rendering_buffer rbuf_src((unsigned char*)source.getBytes(), source.width(), source.height(), source.width() * 4);
    pixfmt_pre pixf_src(rbuf_src);
    using img_src_type = agg::image_accessor_clone<pixfmt_pre>;
    img_src_type img_src(pixf_src);

    // initialize destination AGG buffer (with transparency)
    agg::rendering_buffer rbuf_dst((unsigned char*)target.getBytes(), target.width(), target.height(), target.width() * 4);
    pixfmt_pre pixf_dst(rbuf_dst);
    renderer_base_pre rb_dst_pre(pixf_dst);

    // create a scaling matrix
    agg::trans_affine img_mtx;
    img_mtx /= agg::trans_affine_scaling(image_ratio_x, image_ratio_y);

    // create a linear interpolator for our scaling matrix
    using interpolator_type = agg::span_interpolator_linear<>;
    interpolator_type interpolator(img_mtx);

    // draw an anticlockwise polygon to render our image into
    double scaled_width = target.width();
    double scaled_height = target.height();
    ras.reset();
    ras.move_to_d(x_off_f,                y_off_f);
    ras.line_to_d(x_off_f + scaled_width, y_off_f);
    ras.line_to_d(x_off_f + scaled_width, y_off_f + scaled_height);
    ras.line_to_d(x_off_f,                y_off_f + scaled_height);

    switch(scaling_method)
    {
    case SCALING_NEAR:
    {
        using span_gen_type = agg::span_image_filter_rgba_nn<img_src_type, interpolator_type>;
        span_gen_type sg(img_src, interpolator);
        agg::render_scanlines_aa(ras, sl, rb_dst_pre, sa, sg);
        return;
    }
    case SCALING_BILINEAR:
        filter.calculate(agg::image_filter_bilinear(), true); break;
    case SCALING_BICUBIC:
        filter.calculate(agg::image_filter_bicubic(), true); break;
    case SCALING_SPLINE16:
        filter.calculate(agg::image_filter_spline16(), true); break;
    case SCALING_SPLINE36:
        filter.calculate(agg::image_filter_spline36(), true); break;
    case SCALING_HANNING:
        filter.calculate(agg::image_filter_hanning(), true); break;
    case SCALING_HAMMING:
        filter.calculate(agg::image_filter_hamming(), true); break;
    case SCALING_HERMITE:
        filter.calculate(agg::image_filter_hermite(), true); break;
    case SCALING_KAISER:
        filter.calculate(agg::image_filter_kaiser(), true); break;
    case SCALING_QUADRIC:
        filter.calculate(agg::image_filter_quadric(), true); break;
    case SCALING_CATROM:
        filter.calculate(agg::image_filter_catrom(), true); break;
    case SCALING_GAUSSIAN:
        filter.calculate(agg::image_filter_gaussian(), true); break;
    case SCALING_BESSEL:
        filter.calculate(agg::image_filter_bessel(), true); break;
    case SCALING_MITCHELL:
        filter.calculate(agg::image_filter_mitchell(), true); break;
    case SCALING_SINC:
        filter.calculate(agg::image_filter_sinc(filter_factor), true); break;
    case SCALING_LANCZOS:
        filter.calculate(agg::image_filter_lanczos(filter_factor), true); break;
    case SCALING_BLACKMAN:
        filter.calculate(agg::image_filter_blackman(filter_factor), true); break;
    }
    // details on various resampling considerations
    // http://old.nabble.com/Re%3A-Newbie---texture-p5057255.html

    // high quality resampler
    using span_gen_type = agg::span_image_resample_rgba_affine<img_src_type>;

    // faster, lower quality
    //using span_gen_type = agg::span_image_filter_rgba<img_src_type,interpolator_type>;

    // local, modified agg::span_image_resample_rgba_affine
    // dating back to when we were not handling alpha correctly
    // and this file helped work around symptoms
    // https://github.com/mapnik/mapnik/issues/1489
    //using span_gen_type = mapnik::span_image_resample_rgba_affine<img_src_type>;
    span_gen_type sg(img_src, interpolator, filter);
    agg::render_scanlines_aa(ras, sl, rb_dst_pre, sa, sg);
}

template void scale_image_agg<image_data_32>(image_data_32& target,
                                             const image_data_32& source,
                                             scaling_method_e scaling_method,
                                             double image_ratio_x,
                                             double image_ratio_y,
                                             double x_off_f,
                                             double y_off_f,
                                             double filter_factor);

}
