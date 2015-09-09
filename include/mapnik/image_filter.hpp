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


#ifndef MAPNIK_IMAGE_FILTER_HPP
#define MAPNIK_IMAGE_FILTER_HPP

//mapnik
#include <mapnik/image_filter_types.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/util/hsl.hpp>

// boost GIL
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wc++11-narrowing"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#include <boost/gil/gil_all.hpp>
#pragma GCC diagnostic pop

// agg
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_color_rgba.h"
#include "agg_pixfmt_rgba.h"
#include "agg_scanline_u.h"
#include "agg_blur.h"
#include "agg_gradient_lut.h"
// stl
#include <cmath>

// 8-bit YUV
//Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16
//U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128
//V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128

//bits_type x_gradient = (0.125f*c2 + 0.25f*c5 + 0.125f*c8)
//    - (0.125f*c0 + 0.25f*c3 + 0.125f*c6);
//bits_type y_gradient = (0.125f*c0 + 0.25f*c1 + 0.125f*c2)
//    - (0.125f*c6 + 0.25f*c7 + 0.125f*c8);

// c0 c1 c2
// c3 c4 c5
// c6 c7 c8

//sharpen
//  0 -1  0
// -1  5 -1
//  0 -1  0
//bits_type out_value = -c1 - c3 + 5.0*c4 - c5 - c7;

// edge detect
//  0  1  0
//  1 -4  1
//  0  1  0
//bits_type out_value = c1 + c3 - 4.0*c4 + c5 + c7;

//
//if (out_value < 0) out_value = 0;
//if (out_value > 255) out_value = 255;

// emboss
// -2 -1  0
// -1  1  1
//  0  1  2

// bits_type out_value = -2*c0 - c1 - c3 + c4 + c5 + c7 +  2*c8;

// blur
//float out_value = (0.1f*c0 + 0.1f*c1 + 0.1f*c2 +
//                   0.1f*c3 + 0.1f*c4 + 0.1f*c5 +
//                  0.1f*c6 + 0.1f*c7 + 0.1f*c8);


//float out_value  = std::sqrt(std::pow(x_gradient,2) + std::pow(y_gradient,2));
//float theta = std::atan2(x_gradient,y_gradient);
//if (out_value < 0.0) out_value = 0.0;
//if (out_value < 1.0) out_value = 1.0;




//float conv_matrix[]={1/3.0,1/3.0,1/3.0};

//float gaussian_1[]={0.00022923296f,0.0059770769f,0.060597949f,0.24173197f,0.38292751f,
//                    0.24173197f,0.060597949f,0.0059770769f,0.00022923296f};

//float gaussian_2[]={
//    0.00048869418f,0.0024031631f,0.0092463447f,
//   0.027839607f,0.065602221f,0.12099898f,0.17469721f,
//   0.19744757f,
//   0.17469721f,0.12099898f,0.065602221f,0.027839607f,
//   0.0092463447f,0.0024031631f,0.00048869418f
//};

//  kernel_1d_fixed<float,9> kernel(conv,4);

// color_converted_view<rgb8_pixel_t>(src_view);
//using view_t = kth_channel_view_type< 0, const rgba8_view_t>::type;

//view_t red = kth_channel_view<0>(const_view(src_view));

//kernel_1d_fixed<float,3> kernel(sharpen,0);
//convolve_rows_fixed<rgba32f_pixel_t>(src_view,kernel,src_view);
// convolve_cols_fixed<rgba32f_pixel_t>(src_view,kernel,dst_view);

namespace mapnik {  namespace filter { namespace detail {

static const float blur_matrix[] = {0.1111f,0.1111f,0.1111f,0.1111f,0.1111f,0.1111f,0.1111f,0.1111f,0.1111f};
static const float emboss_matrix[] = {-2,-1,0,-1,1,1,0,1,2};
static const float sharpen_matrix[] = {0,-1,0,-1,5,-1,0,-1,0 };
static const float edge_detect_matrix[] = {0,1,0,1,-4,1,0,1,0 };

}

using boost::gil::rgba8_image_t;
using boost::gil::rgba8_view_t;

template <typename Image>
boost::gil::rgba8_view_t rgba8_view(Image & img)
{
    using boost::gil::interleaved_view;
    using boost::gil::rgba8_pixel_t;
    return interleaved_view(img.width(), img.height(),
                            reinterpret_cast<rgba8_pixel_t*>(img.bytes()),
                            img.width() * sizeof(rgba8_pixel_t));
}

template <typename Image>
struct double_buffer
{
    boost::gil::rgba8_image_t   dst_buffer;
    boost::gil::rgba8_view_t    dst_view;
    boost::gil::rgba8_view_t    src_view;

    explicit double_buffer(Image & src)
        : dst_buffer(src.width(), src.height())
        , dst_view(view(dst_buffer))
        , src_view(rgba8_view(src)) {}

    ~double_buffer()
    {
        copy_pixels(dst_view, src_view);
    }
};

template <typename Src, typename Dst, typename Conv>
void process_channel_impl (Src const& src, Dst & dst, Conv const& k)
{
    using boost::gil::bits32f;

    bits32f out_value =
        k[0]*src[0] + k[1]*src[1] + k[2]*src[2] +
        k[3]*src[3] + k[4]*src[4] + k[5]*src[5] +
        k[6]*src[6] + k[7]*src[7] + k[8]*src[8]
        ;
    if (out_value < 0) out_value = 0;
    if (out_value > 255) out_value = 255;
    dst = out_value;
}

template <typename Src, typename Dst, typename Conv>
void process_channel (Src const&, Dst &, Conv const&)
{
}

template <typename Src, typename Dst>
void process_channel (Src const& src, Dst & dst, mapnik::filter::blur)
{
    process_channel_impl(src,dst,mapnik::filter::detail::blur_matrix);
}

template <typename Src, typename Dst>
void process_channel (Src const& src, Dst & dst, mapnik::filter::emboss)
{
    process_channel_impl(src,dst,mapnik::filter::detail::emboss_matrix);
}

template <typename Src, typename Dst>
void process_channel (Src const& src, Dst & dst, mapnik::filter::sharpen)
{
    process_channel_impl(src,dst,mapnik::filter::detail::sharpen_matrix);
}

template <typename Src, typename Dst>
void process_channel (Src const& src, Dst & dst, mapnik::filter::edge_detect)
{
    process_channel_impl(src,dst,mapnik::filter::detail::edge_detect_matrix);
}


template <typename Src, typename Dst>
void process_channel (Src const& src, Dst & dst, mapnik::filter::sobel)
{
    using boost::gil::bits32f;

    bits32f x_gradient = (src[2] + 2*src[5] + src[8])
        - (src[0] + 2*src[3] + src[6]);

    bits32f y_gradient = (src[0] + 2*src[1] + src[2])
        - (src[6] + 2*src[7] + src[8]);

    bits32f  out_value  = std::sqrt(std::pow(x_gradient,2) + std::pow(y_gradient,2));
    //bts32f theta = std::atan2(x_gradient,y_gradient);
    if (out_value < 0) out_value = 0;
    if (out_value > 255) out_value = 255;
    dst = out_value;
}



template <typename Src, typename Dst, typename Filter>
void apply_convolution_3x3(Src const& src_view, Dst & dst_view, Filter const& filter)
{
    using boost::gil::bits32f;
    using boost::gil::point2;

    // p0 p1 p2
    // p3 p4 p5
    // p6 p7 p8

    typename Src::xy_locator src_loc = src_view.xy_at(0,0);
    typename Src::xy_locator::cached_location_t loc00 = src_loc.cache_location(-1,-1);
    typename Src::xy_locator::cached_location_t loc10 = src_loc.cache_location( 0,-1);
    typename Src::xy_locator::cached_location_t loc20 = src_loc.cache_location( 1,-1);
    typename Src::xy_locator::cached_location_t loc01 = src_loc.cache_location(-1, 0);
    typename Src::xy_locator::cached_location_t loc11 = src_loc.cache_location( 0, 0);
    typename Src::xy_locator::cached_location_t loc21 = src_loc.cache_location( 1, 0);
    typename Src::xy_locator::cached_location_t loc02 = src_loc.cache_location(-1, 1);
    typename Src::xy_locator::cached_location_t loc12 = src_loc.cache_location( 0, 1);
    typename Src::xy_locator::cached_location_t loc22 = src_loc.cache_location( 1, 1);

    typename Src::x_iterator dst_it = dst_view.row_begin(0);

    // top row
    for (std::ptrdiff_t x = 0 ; x < src_view.width(); ++x)
    {
        (*dst_it)[3] = src_loc[loc11][3]; // Dst.a = Src.a
        for (std::ptrdiff_t i = 0; i < 3; ++i)
        {
            bits32f p[9];

            p[4] = src_loc[loc11][i];
            p[7] = src_loc[loc12][i];

            if (x == 0)
            {
                p[3] = p[4];
                p[6] = p[7];
            }
            else
            {
                p[3] = src_loc[loc01][i];
                p[6] = src_loc[loc02][i];
            }

            if ( x == (src_view.width())-1)
            {
                p[5] = p[4];
                p[8] = p[7];
            }
            else
            {
                p[5] = src_loc[loc21][i];
                p[8] = src_loc[loc22][i];
            }

            p[0] = p[6];
            p[1] = p[7];
            p[2] = p[8];

            process_channel(p, (*dst_it)[i], filter);
        }
        ++src_loc.x();
        ++dst_it;
    }
    // carrige-return
    src_loc += point2<std::ptrdiff_t>(-src_view.width(),1);

    // 1... height-1 rows
    for (std::ptrdiff_t y = 1; y < src_view.height()-1; ++y)
    {
        for (std::ptrdiff_t x = 0; x < src_view.width(); ++x)
        {
            (*dst_it)[3] = src_loc[loc11][3]; // Dst.a = Src.a
            for (std::ptrdiff_t i = 0; i < 3; ++i)
            {
                bits32f p[9];

                p[1] = src_loc[loc10][i];
                p[4] = src_loc[loc11][i];
                p[7] = src_loc[loc12][i];

                if (x == 0)
                {
                    p[0] = p[1];
                    p[3] = p[4];
                    p[6] = p[7];
                }
                else
                {
                    p[0] = src_loc[loc00][i];
                    p[3] = src_loc[loc01][i];
                    p[6] = src_loc[loc02][i];
                }

                if ( x == (src_view.width()) - 1)
                {
                    p[2] = p[1];
                    p[5] = p[4];
                    p[8] = p[7];
                }
                else
                {
                    p[2] = src_loc[loc20][i];
                    p[5] = src_loc[loc21][i];
                    p[8] = src_loc[loc22][i];
                }
                process_channel(p, (*dst_it)[i], filter);
            }
            ++dst_it;
            ++src_loc.x();
        }
        // carrige-return
        src_loc += point2<std::ptrdiff_t>(-src_view.width(),1);
    }

    // bottom row
    //src_loc = src_view.xy_at(0,src_view.height()-1);
    for (std::ptrdiff_t x = 0 ; x < src_view.width(); ++x)
    {
        (*dst_it)[3] = src_loc[loc11][3]; // Dst.a = Src.a
        for (std::ptrdiff_t i = 0; i < 3; ++i)
        {
            bits32f p[9];

            p[1] = src_loc[loc10][i];
            p[4] = src_loc[loc11][i];

            if (x == 0)
            {
                p[0] = p[1];
                p[3] = p[4];
            }
            else
            {
                p[0] = src_loc[loc00][i];
                p[3] = src_loc[loc01][i];
            }

            if ( x == (src_view.width())-1)
            {
                p[2] = p[1];
                p[5] = p[4];

            }
            else
            {
                p[2] = src_loc[loc20][i];
                p[5] = src_loc[loc21][i];
            }

            p[6] = p[0];
            p[7] = p[1];
            p[8] = p[2];
            process_channel(p, (*dst_it)[i], filter);
        }
        ++src_loc.x();
        ++dst_it;
    }
}

template <typename Src, typename Filter>
void apply_filter(Src & src, Filter const& filter)
{
    demultiply_alpha(src);
    double_buffer<Src> tb(src);
    apply_convolution_3x3(tb.src_view, tb.dst_view, filter);
}

template <typename Src>
void apply_filter(Src & src, agg_stack_blur const& op)
{
    premultiply_alpha(src);
    agg::rendering_buffer buf(src.bytes(),src.width(),src.height(), src.row_size());
    agg::pixfmt_rgba32_pre pixf(buf);
    agg::stack_blur_rgba32(pixf,op.rx,op.ry);
}

inline double channel_delta(double source, double match)
{
    if (source > match) return (source - match) / (1.0 - match);
    if (source < match) return (match - source) / match;
    return (source - match);
}

inline uint8_t apply_alpha_shift(double source, double match, double alpha)
{
    source = (((source - match) / alpha) + match) * alpha;
    return static_cast<uint8_t>(std::floor((source*255.0)+.5));
}

template <typename Src>
void apply_filter(Src & src, color_to_alpha const& op)
{
    using namespace boost::gil;
    bool premultiplied = src.get_premultiplied();
    rgba8_view_t src_view = rgba8_view(src);
    double cr = static_cast<double>(op.color.red())/255.0;
    double cg = static_cast<double>(op.color.green())/255.0;
    double cb = static_cast<double>(op.color.blue())/255.0;
    for (std::ptrdiff_t y = 0; y < src_view.height(); ++y)
    {
        rgba8_view_t::x_iterator src_it = src_view.row_begin(static_cast<long>(y));
        for (std::ptrdiff_t x = 0; x < src_view.width(); ++x)
        {
            uint8_t & r = get_color(src_it[x], red_t());
            uint8_t & g = get_color(src_it[x], green_t());
            uint8_t & b = get_color(src_it[x], blue_t());
            uint8_t & a = get_color(src_it[x], alpha_t());
            double sr = static_cast<double>(r)/255.0;
            double sg = static_cast<double>(g)/255.0;
            double sb = static_cast<double>(b)/255.0;
            double sa = static_cast<double>(a)/255.0;
            // demultiply
            if (sa <= 0.0)
            {
                r = g = b = 0;
                continue;
            }
            else if (premultiplied)
            {
                sr /= sa;
                sg /= sa;
                sb /= sa;
            }
            // get that maximum color difference
            double xa = std::max(channel_delta(sr,cr),std::max(channel_delta(sg,cg),channel_delta(sb,cb)));
            if (xa > 0)
            {
                // apply difference to each channel, returning premultiplied
                // TODO - experiment with difference in hsl color space
                r = apply_alpha_shift(sr,cr,xa);
                g = apply_alpha_shift(sg,cg,xa);
                b = apply_alpha_shift(sb,cb,xa);
                // combine new alpha with original
                xa *= sa;
                a = static_cast<uint8_t>(std::floor((xa*255.0)+.5));
                // all color values must be <= alpha
                if (r>a) r=a;
                if (g>a) g=a;
                if (b>a) b=a;
            }
            else
            {
                r = g = b = a = 0;
            }
        }
    }
    // set as premultiplied
    set_premultiplied_alpha(src, true);
}

template <typename Src>
void apply_filter(Src & src, colorize_alpha const& op)
{
    using namespace boost::gil;
    std::ptrdiff_t size = op.size();
    if (op.size() == 1)
    {
        // no interpolation if only one stop
        mapnik::filter::color_stop const& stop = op[0];
        mapnik::color const& c = stop.color;
        rgba8_view_t src_view = rgba8_view(src);
        for (std::ptrdiff_t y = 0; y < src_view.height(); ++y)
        {
            rgba8_view_t::x_iterator src_it = src_view.row_begin(static_cast<long>(y));
            for (std::ptrdiff_t x = 0; x < src_view.width(); ++x)
            {
                uint8_t & r = get_color(src_it[x], red_t());
                uint8_t & g = get_color(src_it[x], green_t());
                uint8_t & b = get_color(src_it[x], blue_t());
                uint8_t & a = get_color(src_it[x], alpha_t());
                if ( a > 0)
                {
                    a = (c.alpha() * a + 255) >> 8;
                    r = (c.red() * a + 255) >> 8;
                    g = (c.green() * a + 255) >> 8;
                    b = (c.blue() * a + 255) >> 8;
                }
            }
        }
        // set as premultiplied
        set_premultiplied_alpha(src, true);
    }
    else if (size > 1)
    {
        // interpolate multiple stops
        agg::gradient_lut<agg::color_interpolator<agg::rgba8> > grad_lut;
        double step = 1.0/(size-1);
        double offset = 0.0;
        for ( mapnik::filter::color_stop const& stop : op)
        {
            mapnik::color const& c = stop.color;
            double stop_offset = stop.offset;
            if (stop_offset == 0)
            {
                stop_offset = offset;
            }
            grad_lut.add_color(stop_offset, agg::rgba(c.red()/255.0,
                                                      c.green()/255.0,
                                                      c.blue()/255.0,
                                                      c.alpha()/255.0));
            offset += step;
        }
        if (grad_lut.build_lut())
        {
            rgba8_view_t src_view = rgba8_view(src);
            for (std::ptrdiff_t y = 0; y < src_view.height(); ++y)
            {
                rgba8_view_t::x_iterator src_it = src_view.row_begin(static_cast<long>(y));
                for (std::ptrdiff_t x = 0; x < src_view.width(); ++x)
                {
                    uint8_t & r = get_color(src_it[x], red_t());
                    uint8_t & g = get_color(src_it[x], green_t());
                    uint8_t & b = get_color(src_it[x], blue_t());
                    uint8_t & a = get_color(src_it[x], alpha_t());
                    if ( a > 0)
                    {
                        agg::rgba8 c = grad_lut[a];
                        a = (c.a * a + 255) >> 8;
                        r = (c.r * a + 255) >> 8;
                        g = (c.g * a + 255) >> 8;
                        b = (c.b * a + 255) >> 8;
        #if 0
                        // rainbow
                        r = 0;
                        g = 0;
                        b = 0;
                        if (a < 64)
                        {
                            g = a * 4;
                            b = 255;
                        }
                        else if (a >= 64 && a < 128)
                        {
                            g = 255;
                            b = 255 - ((a - 64) * 4);
                        }
                        else if (a >= 128 && a < 192)
                        {
                            r = (a - 128) * 4;
                            g = 255;
                        }
                        else // >= 192
                        {
                            r = 255;
                            g = 255 - ((a - 192) * 4);
                        }
                        r = (r * a + 255) >> 8;
                        g = (g * a + 255) >> 8;
                        b = (b * a + 255) >> 8;
        #endif
                    }
                }
            }
        }
        // set as premultiplied
        set_premultiplied_alpha(src, true);
    }
}

template <typename Src>
void apply_filter(Src & src, scale_hsla const& transform)
{
    using namespace boost::gil;
    bool tinting = !transform.is_identity();
    bool set_alpha = !transform.is_alpha_identity();
    // todo - filters be able to report if they
    // should be run to avoid overhead of temp buffer
    if (tinting || set_alpha)
    {
        bool premultiplied = src.get_premultiplied();
        rgba8_view_t src_view = rgba8_view(src);
        for (std::ptrdiff_t y = 0; y < src_view.height(); ++y)
        {
            rgba8_view_t::x_iterator src_it = src_view.row_begin(static_cast<long>(y));
            for (std::ptrdiff_t x = 0; x < src_view.width(); ++x)
            {
                uint8_t & r = get_color(src_it[x], red_t());
                uint8_t & g = get_color(src_it[x], green_t());
                uint8_t & b = get_color(src_it[x], blue_t());
                uint8_t & a = get_color(src_it[x], alpha_t());
                double r2 = static_cast<double>(r)/255.0;
                double g2 = static_cast<double>(g)/255.0;
                double b2 = static_cast<double>(b)/255.0;
                double a2 = static_cast<double>(a)/255.0;
                // demultiply
                if (a2 <= 0.0)
                {
                    r = g = b = 0;
                    continue;
                }
                else if (premultiplied)
                {
                    r2 /= a2;
                    g2 /= a2;
                    b2 /= a2;
                }

                if (set_alpha)
                {
                    a2 = transform.a0 + (a2 * (transform.a1 - transform.a0));
                    if (a2 <= 0)
                    {
                        r = g = b = a = 0;
                        continue;
                    }
                    else if (a2 > 1)
                    {
                        a2 = 1;
                        a = 255;
                    }
                    else
                    {
                        a = static_cast<uint8_t>(std::floor((a2 * 255.0) +.5));
                    }
                }
                if (tinting)
                {
                    double h;
                    double s;
                    double l;
                    rgb2hsl(r2,g2,b2,h,s,l);
                    double h2 = transform.h0 + (h * (transform.h1 - transform.h0));
                    double s2 = transform.s0 + (s * (transform.s1 - transform.s0));
                    double l2 = transform.l0 + (l * (transform.l1 - transform.l0));
                    if (h2 > 1) { h2 = 1; }
                    else if (h2 < 0) { h2 = 0; }
                    if (s2 > 1) { s2 = 1; }
                    else if (s2 < 0) { s2 = 0; }
                    if (l2 > 1) { l2 = 1; }
                    else if (l2 < 0) { l2 = 0; }
                    hsl2rgb(h2,s2,l2,r2,g2,b2);
                }
                // premultiply
                r2 *= a2;
                g2 *= a2;
                b2 *= a2;
                r = static_cast<uint8_t>(std::floor((r2*255.0)+.5));
                g = static_cast<uint8_t>(std::floor((g2*255.0)+.5));
                b = static_cast<uint8_t>(std::floor((b2*255.0)+.5));
                // all color values must be <= alpha
                if (r>a) r=a;
                if (g>a) g=a;
                if (b>a) b=a;
            }
        }
        // set as premultiplied
        set_premultiplied_alpha(src, true);
    }
}

template <typename Src, typename ColorBlindFilter>
void color_blind_filter(Src & src, ColorBlindFilter const& op)
{
    using namespace boost::gil;
    rgba8_view_t src_view = rgba8_view(src);
    bool premultiplied = src.get_premultiplied();
    
    for (std::ptrdiff_t y = 0; y < src_view.height(); ++y)
    {
        rgba8_view_t::x_iterator src_it = src_view.row_begin(static_cast<long>(y));
        for (std::ptrdiff_t x = 0; x < src_view.width(); ++x)
        {
            // formula taken from boost/gil/color_convert.hpp:rgb_to_luminance
            uint8_t & r = get_color(src_it[x], red_t());
            uint8_t & g = get_color(src_it[x], green_t());
            uint8_t & b = get_color(src_it[x], blue_t());
            uint8_t & a = get_color(src_it[x], alpha_t());
            double dr = static_cast<double>(r)/255.0;
            double dg = static_cast<double>(g)/255.0;
            double db = static_cast<double>(b)/255.0;
            double da = static_cast<double>(a)/255.0;
            // demultiply
            if (da <= 0.0)
            {
                r = g = b = 0;
                continue;
            }
            else if (premultiplied)
            {
                dr /= da;
                dg /= da;
                db /= da;
            }
            // Convert source color into XYZ color space
            double pow_r = std::pow(dr, 2.2);
            double pow_g = std::pow(dg, 2.2);
            double pow_b = std::pow(db, 2.2);
            double X = (0.412424 * pow_r) + (0.357579 * pow_g) + (0.180464 * pow_b);
            double Y = (0.212656 * pow_r) + (0.715158 * pow_g) + (0.0721856 * pow_b);
            double Z = (0.0193324 * pow_r) + (0.119193 * pow_g) + (0.950444 * pow_b);
            // Convert XYZ into xyY Chromacity Coordinates (xy) and Luminance (Y)
            double chroma_x = X / (X + Y + Z);
            double chroma_y = Y / (X + Y + Z);
            // Generate the "Confusion Line" between the source color and the Confusion Point
            double m_div = chroma_x - op.x;
            if (std::abs(m_div) < (std::numeric_limits<double>::epsilon())) continue;
            double m = (chroma_y - op.y) / (chroma_x - op.x); // slope of Confusion Line
            double yint = chroma_y - chroma_x * m; // y-intercept of confusion line (x-intercept = 0.0)
            // How far the xy coords deviate from the simulation
            double m_div2 = m - op.m;
            if (std::abs(m_div2) < (std::numeric_limits<double>::epsilon())) continue;
            double deviate_x = (op.yint - yint) / (m - op.m);
            double deviate_y = (m * deviate_x) + yint;
            if (std::abs(deviate_y) < (std::numeric_limits<double>::epsilon()))
            {
                deviate_y = std::numeric_limits<double>::epsilon() * 2.0;
            }
            // Compute the simulated color's XYZ coords
            X = deviate_x * Y / deviate_y;
            Z = (1.0 - (deviate_x + deviate_y)) * Y / deviate_y;
            // Neutral grey calculated from luminance (in D65)
            double neutral_X = 0.312713 * Y / 0.329016; 
            double neutral_Z = 0.358271 * Y / 0.329016;
            // Difference between simulated color and neutral grey
            double diff_X = neutral_X - X;
            double diff_Z = neutral_Z - Z;
            double diff_r = diff_X * 3.24071 + diff_Z * -0.498571; // XYZ->RGB (sRGB:D65)
            double diff_g = diff_X * -0.969258 + diff_Z * 0.0415557;
            double diff_b = diff_X * 0.0556352 + diff_Z * 1.05707;
            if (std::abs(diff_r) < (std::numeric_limits<double>::epsilon()))
            {
                diff_r = std::numeric_limits<double>::epsilon() * 2.0;
            }
            if (std::abs(diff_g) < (std::numeric_limits<double>::epsilon()))
            {
                diff_g = std::numeric_limits<double>::epsilon() * 2.0;
            }
            if (std::abs(diff_b) < (std::numeric_limits<double>::epsilon()))
            {
                diff_b = std::numeric_limits<double>::epsilon() * 2.0;
            }
            // Convert to RGB color space
            dr = X * 3.24071 + Y * -1.53726 + Z * -0.498571; // XYZ->RGB (sRGB:D65)
            dg = X * -0.969258 + Y * 1.87599 + Z * 0.0415557;
            db = X * 0.0556352 + Y * -0.203996 + Z * 1.05707;            
            // Compensate simulated color towards a neutral fit in RGB space
            double fit_r = ((dr < 0.0 ? 0.0 : 1.0) - dr) / diff_r;
            double fit_g = ((dg < 0.0 ? 0.0 : 1.0) - dg) / diff_g;
            double fit_b = ((db < 0.0 ? 0.0 : 1.0) - db) / diff_b;
            double adjust = std::max( (fit_r > 1.0 || fit_r < 0.0) ? 0.0 : fit_r, 
                                      (fit_g > 1.0 || fit_g < 0.0) ? 0.0 : fit_g
                                    );
            adjust = std::max((fit_b > 1.0 || fit_b < 0.0) ? 0.0 : fit_b, adjust);
            // Shift proportional to the greatest shift
            dr = dr + (adjust * diff_r);
            dg = dg + (adjust * diff_g);
            db = db + (adjust * diff_b);
            // Apply gamma correction
            dr = std::pow(dr, 1.0 / 2.2);
            dg = std::pow(dg, 1.0 / 2.2);
            db = std::pow(db, 1.0 / 2.2);            
            // premultiply
            dr *= da;
            dg *= da;
            db *= da;
            // Clamp values
            if(dr < 0.0)  dr = 0.0;
            if(dr > 1.0) dr = 1.0;
            if(dg < 0.0) dg = 0.0;
            if(dg > 1.0) dg = 1.0;
            if(db < 0.0) db = 0.0;
            if(db > 1.0) db = 1.0;
            r = static_cast<uint8_t>(dr * 255.0);
            g = static_cast<uint8_t>(dg * 255.0);
            b = static_cast<uint8_t>(db * 255.0);
        }
    }
    // set as premultiplied
    set_premultiplied_alpha(src, true);
}

template <typename Src>
void apply_filter(Src & src, color_blind_protanope const& op)
{
    color_blind_filter(src, op);
}

template <typename Src>
void apply_filter(Src & src, color_blind_deuteranope const& op)
{
    color_blind_filter(src, op);
}

template <typename Src>
void apply_filter(Src & src, color_blind_tritanope const& op)
{
    color_blind_filter(src, op);
}

template <typename Src>
void apply_filter(Src & src, gray const& /*op*/)
{
    premultiply_alpha(src);
    using namespace boost::gil;

    rgba8_view_t src_view = rgba8_view(src);

    for (std::ptrdiff_t y = 0; y < src_view.height(); ++y)
    {
        rgba8_view_t::x_iterator src_it = src_view.row_begin(static_cast<long>(y));
        for (std::ptrdiff_t x = 0; x < src_view.width(); ++x)
        {
            // formula taken from boost/gil/color_convert.hpp:rgb_to_luminance
            uint8_t & r = get_color(src_it[x], red_t());
            uint8_t & g = get_color(src_it[x], green_t());
            uint8_t & b = get_color(src_it[x], blue_t());
            uint8_t   v = uint8_t((4915 * r + 9667 * g + 1802 * b + 8192) >> 14);
            r = g = b = v;
        }
    }
}

template <typename Src, typename Dst>
void x_gradient_impl(Src const& src_view, Dst const& dst_view)
{
    for (std::ptrdiff_t y = 0; y < src_view.height(); ++y)
    {
        typename Src::x_iterator src_it = src_view.row_begin(static_cast<long>(y));
        typename Dst::x_iterator dst_it = dst_view.row_begin(static_cast<long>(y));

        dst_it[0][0] = 128 + (src_it[0][0] - src_it[1][0]) / 2;
        dst_it[0][1] = 128 + (src_it[0][1] - src_it[1][1]) / 2;
        dst_it[0][2] = 128 + (src_it[0][2] - src_it[1][2]) / 2;

        dst_it[dst_view.width()-1][0] = 128 + (src_it[(src_view.width())-2][0] - src_it[(src_view.width())-1][0]) / 2;
        dst_it[dst_view.width()-1][1] = 128 + (src_it[(src_view.width())-2][1] - src_it[(src_view.width())-1][1]) / 2;
        dst_it[dst_view.width()-1][2] = 128 + (src_it[(src_view.width())-2][2] - src_it[(src_view.width())-1][2]) / 2;

        dst_it[0][3] = dst_it[(src_view.width())-1][3] = 255;

        for (std::ptrdiff_t x = 1; x < src_view.width()-1; ++x)
        {
            dst_it[x][0] = 128 + (src_it[x-1][0] - src_it[x+1][0]) / 2;
            dst_it[x][1] = 128 + (src_it[x-1][1] - src_it[x+1][1]) / 2;
            dst_it[x][2] = 128 + (src_it[x-1][2] - src_it[x+1][2]) / 2;
            dst_it[x][3] = 255;
        }
    }
}

template <typename Src>
void apply_filter(Src & src, x_gradient const& /*op*/)
{
    premultiply_alpha(src);
    double_buffer<Src> tb(src);
    x_gradient_impl(tb.src_view, tb.dst_view);
}

template <typename Src>
void apply_filter(Src & src, y_gradient const& /*op*/)
{
    premultiply_alpha(src);
    double_buffer<Src> tb(src);
    x_gradient_impl(rotated90ccw_view(tb.src_view),
                    rotated90ccw_view(tb.dst_view));
}

template <typename Src>
void apply_filter(Src & src, invert const& /*op*/)
{
    premultiply_alpha(src);
    using namespace boost::gil;

    rgba8_view_t src_view = rgba8_view(src);

    for (std::ptrdiff_t y = 0; y < src_view.height(); ++y)
    {
        rgba8_view_t::x_iterator src_it = src_view.row_begin(static_cast<long>(y));
        for (std::ptrdiff_t x = 0; x < src_view.width(); ++x)
        {
            // we only work with premultiplied source,
            // thus all color values must be <= alpha
            uint8_t   a = get_color(src_it[x], alpha_t());
            uint8_t & r = get_color(src_it[x], red_t());
            uint8_t & g = get_color(src_it[x], green_t());
            uint8_t & b = get_color(src_it[x], blue_t());
            r = a - r;
            g = a - g;
            b = a - b;
        }
    }
}

template <typename Src>
struct filter_visitor
{
    filter_visitor(Src & src)
    : src_(src) {}

    template <typename T>
    void operator () (T const& filter)
    {
        apply_filter(src_, filter);
    }

    Src & src_;
};

struct filter_radius_visitor
{
    int & radius_;
    filter_radius_visitor(int & radius)
        : radius_(radius) {}
    template <typename T>
    void operator () (T const& /*filter*/) {}

    void operator () (agg_stack_blur const& op)
    {
        if (static_cast<int>(op.rx) > radius_) radius_ = static_cast<int>(op.rx);
        if (static_cast<int>(op.ry) > radius_) radius_ = static_cast<int>(op.ry);
    }
};

template<typename Src>
void filter_image(Src & src, std::string const& filter)
{
    std::vector<filter_type> filter_vector;
    if(!parse_image_filters(filter, filter_vector))
    {
        throw std::runtime_error("Failed to parse filter argument in filter_image: '" + filter + "'");
    }
    filter_visitor<Src> visitor(src);
    for (filter_type const& filter_tag : filter_vector)
    {
        util::apply_visitor(visitor, filter_tag);
    }
}

template<typename Src>
Src filter_image(Src const& src, std::string const& filter)
{
    std::vector<filter_type> filter_vector;
    if(!parse_image_filters(filter, filter_vector))
    {
        throw std::runtime_error("Failed to parse filter argument in filter_image: '" + filter + "'");
    }
    Src new_src(src);
    filter_visitor<Src> visitor(new_src);
    for (filter_type const& filter_tag : filter_vector)
    {
        util::apply_visitor(visitor, filter_tag);
    }
    return new_src;
}

} // End Namespace Filter

} // End Namespace Mapnik

#endif // MAPNIK_IMAGE_FILTER_HPP
