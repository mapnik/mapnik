/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <mapnik/image.hpp>
#include <mapnik/image_scaling.hpp>
#include <mapnik/image_scaling_traits.hpp>
// does not handle alpha correctly
//#include <mapnik/span_image_filter.hpp>

// boost
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/bimap.hpp>
#pragma GCC diagnostic pop

// agg
#include "agg_image_accessors.h"
#include "agg_pixfmt_rgba.h"
#include "agg_pixfmt_gray.h"
#include "agg_color_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_rendering_buffer.h"
#include "agg_scanline_u.h"
#include "agg_span_allocator.h"
#include "agg_span_image_filter_gray.h"
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


template <typename T>
void scale_image_agg(T & target, T const& source, scaling_method_e scaling_method,
                     double image_ratio_x, double image_ratio_y, double x_off_f, double y_off_f,
                     double filter_factor)
{
    // "the image filters should work namely in the premultiplied color space"
    // http://old.nabble.com/Re:--AGG--Basic-image-transformations-p1110665.html
    // "Yes, you need to use premultiplied images only. Only in this case the simple weighted averaging works correctly in the image fitering."
    // http://permalink.gmane.org/gmane.comp.graphics.agg/3443
    using image_type = T;
    using pixel_type = typename image_type::pixel_type;
    using pixfmt_pre = typename detail::agg_scaling_traits<image_type>::pixfmt_pre;
    using color_type = typename detail::agg_scaling_traits<image_type>::color_type;
    using img_src_type = typename detail::agg_scaling_traits<image_type>::img_src_type;
    using interpolator_type = typename detail::agg_scaling_traits<image_type>::interpolator_type;
    using renderer_base_pre = agg::renderer_base<pixfmt_pre>;
    constexpr std::size_t pixel_size = sizeof(pixel_type);

    // define some stuff we'll use soon
    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_u8 sl;
    agg::span_allocator<color_type> sa;

    // initialize source AGG buffer
    agg::rendering_buffer rbuf_src(const_cast<unsigned char*>(source.bytes()),
                                   source.width(), source.height(), source.width() * pixel_size);
    pixfmt_pre pixf_src(rbuf_src);

    img_src_type img_src(pixf_src);

    // initialize destination AGG buffer (with transparency)
    agg::rendering_buffer rbuf_dst(target.bytes(), target.width(), target.height(), target.width() * pixel_size);
    pixfmt_pre pixf_dst(rbuf_dst);
    renderer_base_pre rb_dst_pre(pixf_dst);

    // create a scaling matrix
    agg::trans_affine img_mtx;
    img_mtx /= agg::trans_affine_scaling(image_ratio_x, image_ratio_y);

    // create a linear interpolator for our scaling matrix
    interpolator_type interpolator(img_mtx);
    // draw an anticlockwise polygon to render our image into
    double scaled_width = target.width();
    double scaled_height = target.height();
    ras.reset();
    ras.move_to_d(x_off_f,                y_off_f);
    ras.line_to_d(x_off_f + scaled_width, y_off_f);
    ras.line_to_d(x_off_f + scaled_width, y_off_f + scaled_height);
    ras.line_to_d(x_off_f,                y_off_f + scaled_height);

    if (scaling_method == SCALING_NEAR)
    {
        using span_gen_type = typename detail::agg_scaling_traits<image_type>::span_image_filter;
        span_gen_type sg(img_src, interpolator);
        agg::render_scanlines_aa(ras, sl, rb_dst_pre, sa, sg);
    }
    else
    {
        using span_gen_type = typename detail::agg_scaling_traits<image_type>::span_image_resample_affine;
        agg::image_filter_lut filter;
        detail::set_scaling_method(filter, scaling_method, filter_factor);
        span_gen_type sg(img_src, interpolator, filter);
        agg::render_scanlines_aa(ras, sl, rb_dst_pre, sa, sg);
    }

}

template MAPNIK_DECL void scale_image_agg(image_rgba8 &, image_rgba8 const&, scaling_method_e,
                              double, double , double, double , double);

template MAPNIK_DECL void scale_image_agg(image_gray8 &, image_gray8 const&, scaling_method_e,
                              double, double , double, double , double);

template MAPNIK_DECL void scale_image_agg(image_gray8s &, image_gray8s const&, scaling_method_e,
                              double, double , double, double , double);

template MAPNIK_DECL void scale_image_agg(image_gray16 &, image_gray16 const&, scaling_method_e,
                              double, double , double, double , double);

template MAPNIK_DECL void scale_image_agg(image_gray16s &, image_gray16s const&, scaling_method_e,
                              double, double , double, double , double);

template MAPNIK_DECL void scale_image_agg(image_gray32 &, image_gray32 const&, scaling_method_e,
                              double, double , double, double , double);

template MAPNIK_DECL void scale_image_agg(image_gray32s &, image_gray32s const&, scaling_method_e,
                              double, double , double, double , double);

template MAPNIK_DECL void scale_image_agg(image_gray32f &, image_gray32f const&, scaling_method_e,
                              double, double , double, double , double);

template MAPNIK_DECL void scale_image_agg(image_gray64 &, image_gray64 const&, scaling_method_e,
                              double, double , double, double , double);

template MAPNIK_DECL void scale_image_agg(image_gray64s &, image_gray64s const&, scaling_method_e,
                              double, double , double, double , double);

template MAPNIK_DECL void scale_image_agg(image_gray64f &, image_gray64f const&, scaling_method_e,
                              double, double , double, double , double);
}
