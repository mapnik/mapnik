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

#ifndef MAPNIK_IMAGE_SCALING_TRAITS_HPP
#define MAPNIK_IMAGE_SCALING_TRAITS_HPP

// mapnik
#include <mapnik/span_image_filter.h>

// agg
#include "agg_image_accessors.h"
#include "agg_pixfmt_rgba.h"
#include "agg_pixfmt_gray.h"
#include "agg_span_allocator.h"
#include "agg_span_image_filter_gray.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_span_interpolator_linear.h"

namespace mapnik  { namespace detail {

template <typename T>
struct agg_scaling_traits  {};

template <>
struct agg_scaling_traits<image_rgba8>
{
    using pixfmt_pre = agg::pixfmt_rgba32_pre;
    using color_type = agg::rgba8;
    using interpolator_type = agg::span_interpolator_linear<>;
    using img_src_type = agg::image_accessor_clone<pixfmt_pre>;
    using span_image_filter = agg::span_image_filter_rgba_nn<img_src_type,interpolator_type>;
    using span_image_resample_affine = span_image_resample_rgba_affine<img_src_type>;

};

template <>
struct agg_scaling_traits<image_gray8>
{
    using pixfmt_pre = agg::pixfmt_gray8_pre;
    using color_type = agg::gray8;
    using interpolator_type = agg::span_interpolator_linear<>;
    using img_src_type = agg::image_accessor_clone<pixfmt_pre>;
    using span_image_filter = agg::span_image_filter_gray_nn<img_src_type,interpolator_type>;
    using span_image_resample_affine = span_image_resample_gray_affine<img_src_type>;
};

template <>
struct agg_scaling_traits<image_gray8s>
{
    using pixfmt_pre = agg::pixfmt_gray8_pre;
    using color_type = agg::gray8;
    using interpolator_type = agg::span_interpolator_linear<>;
    using img_src_type = agg::image_accessor_clone<pixfmt_pre>;
    using span_image_filter = agg::span_image_filter_gray_nn<img_src_type,interpolator_type>;
    using span_image_resample_affine = span_image_resample_gray_affine<img_src_type>;
};

template <>
struct agg_scaling_traits<image_gray16>
{
    using pixfmt_pre = agg::pixfmt_gray16_pre;
    using color_type = agg::gray16;
    using interpolator_type = agg::span_interpolator_linear<>;
    using img_src_type = agg::image_accessor_clone<pixfmt_pre>;
    using span_image_filter = agg::span_image_filter_gray_nn<img_src_type,interpolator_type>;
    using span_image_resample_affine = span_image_resample_gray_affine<img_src_type>;
};

template <>
struct agg_scaling_traits<image_gray16s>
{
    using pixfmt_pre = agg::pixfmt_gray16_pre;
    using color_type = agg::gray16;
    using interpolator_type = agg::span_interpolator_linear<>;
    using img_src_type = agg::image_accessor_clone<pixfmt_pre>;
    using span_image_filter = agg::span_image_filter_gray_nn<img_src_type,interpolator_type>;
    using span_image_resample_affine = span_image_resample_gray_affine<img_src_type>;
};

template <>
struct agg_scaling_traits<image_gray32>
{
    using pixfmt_pre = agg::pixfmt_gray32_pre;
    using color_type = agg::gray32;
    using interpolator_type = agg::span_interpolator_linear<>;
    using img_src_type = agg::image_accessor_clone<pixfmt_pre>;
    using span_image_filter = agg::span_image_filter_gray_nn<img_src_type,interpolator_type>;
    using span_image_resample_affine = span_image_resample_gray_affine<img_src_type>;
};

template <>
struct agg_scaling_traits<image_gray32s>
{
    using pixfmt_pre = agg::pixfmt_gray32_pre;
    using color_type = agg::gray32;
    using interpolator_type = agg::span_interpolator_linear<>;
    using img_src_type = agg::image_accessor_clone<pixfmt_pre>;
    using span_image_filter = agg::span_image_filter_gray_nn<img_src_type,interpolator_type>;
    using span_image_resample_affine = span_image_resample_gray_affine<img_src_type>;
};

template <>
struct agg_scaling_traits<image_gray32f>
{
    using pixfmt_pre = agg::pixfmt_gray32_pre;
    using color_type = agg::gray32;
    using interpolator_type = agg::span_interpolator_linear<>;
    using img_src_type = agg::image_accessor_clone<pixfmt_pre>;
    using span_image_filter = agg::span_image_filter_gray_nn<img_src_type,interpolator_type>;
    using span_image_resample_affine = span_image_resample_gray_affine<img_src_type>;
};

template <>
struct agg_scaling_traits<image_gray64>
{
    using pixfmt_pre = agg::pixfmt_gray32_pre;
    using color_type = agg::gray32;
    using interpolator_type = agg::span_interpolator_linear<>;
    using img_src_type = agg::image_accessor_clone<pixfmt_pre>;
    using span_image_filter = agg::span_image_filter_gray_nn<img_src_type,interpolator_type>;
    using span_image_resample_affine = span_image_resample_gray_affine<img_src_type>;
};

template <>
struct agg_scaling_traits<image_gray64s>
{
    using pixfmt_pre = agg::pixfmt_gray32_pre;
    using color_type = agg::gray32;
    using interpolator_type = agg::span_interpolator_linear<>;
    using img_src_type = agg::image_accessor_clone<pixfmt_pre>;
    using span_image_filter = agg::span_image_filter_gray_nn<img_src_type,interpolator_type>;
    using span_image_resample_affine = span_image_resample_gray_affine<img_src_type>;
};

template <>
struct agg_scaling_traits<image_gray64f>
{
    using pixfmt_pre = agg::pixfmt_gray32_pre;
    using color_type = agg::gray32;
    using interpolator_type = agg::span_interpolator_linear<>;
    using img_src_type = agg::image_accessor_clone<pixfmt_pre>;
    using span_image_filter = agg::span_image_filter_gray_nn<img_src_type,interpolator_type>;
    using span_image_resample_affine = span_image_resample_gray_affine<img_src_type>;
};

template <typename Filter>
void set_scaling_method(Filter & filter, scaling_method_e scaling_method, double filter_factor)
{
    switch(scaling_method)
    {
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
    default:
        break;
    }
}

}

} //


#endif // MAPNIK_IMAGE_SCALING_TRAITS_HPP
