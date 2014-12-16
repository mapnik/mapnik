/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

namespace mapnik  {

namespace detail {

template <typename T>
struct agg_scaling_traits  {};

template <>
struct agg_scaling_traits<image_data_rgba8>
{
    using pixfmt_pre = agg::pixfmt_rgba32_pre;
    using color_type = agg::rgba8;
    using interpolator_type = agg::span_interpolator_linear<>;
    using img_src_type = agg::image_accessor_clone<pixfmt_pre>;
    using span_image_filter = agg::span_image_filter_rgba_nn<img_src_type,interpolator_type>;
    using span_image_resample_affine = agg::span_image_resample_rgba_affine<img_src_type>;

};

template <>
struct agg_scaling_traits<image_data_gray8>
{
    using pixfmt_pre = agg::pixfmt_gray8_pre;
    using color_type = agg::gray8;
    using interpolator_type = agg::span_interpolator_linear<>;
    using img_src_type = agg::image_accessor_clone<pixfmt_pre>;
    using span_image_filter = agg::span_image_filter_gray_nn<img_src_type,interpolator_type>;
    using span_image_resample_affine = agg::span_image_resample_gray_affine<img_src_type>;
};

template <>
struct agg_scaling_traits<image_data_gray16>
{
    using pixfmt_pre = agg::pixfmt_gray16_pre;
    using color_type = agg::gray16;
    using interpolator_type = agg::span_interpolator_linear<>;
    using img_src_type = agg::image_accessor_clone<pixfmt_pre>;
    using span_image_filter = agg::span_image_filter_gray_nn<img_src_type,interpolator_type>;
    using span_image_resample_affine = agg::span_image_resample_gray_affine<img_src_type>;
};

template <>
struct agg_scaling_traits<image_data_gray32f>
{
    using pixfmt_pre = agg::pixfmt_gray32_pre;
    using color_type = agg::gray32;
    using interpolator_type = agg::span_interpolator_linear<>;
    using img_src_type = agg::image_accessor_clone<pixfmt_pre>;
    using span_image_filter = agg::span_image_filter_gray_nn<img_src_type,interpolator_type>;
    using span_image_resample_affine = agg::span_image_resample_gray_affine<img_src_type>;
};

}

} //


#endif // MAPNIK_IMAGE_SCALING_TRAITS_HPP
