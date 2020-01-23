#include "agg_pixfmt_rgba.h"

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION >= 106900
#include <boost/gil.hpp>
#include <boost/gil/extension/toolbox/color_spaces/hsv.hpp>
#include <boost/gil/extension/toolbox/color_spaces/hsl.hpp>
#else
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/toolbox/hsv.hpp>
#include <boost/gil/extension/toolbox/hsl.hpp>
#endif
#pragma GCC diagnostic pop

namespace agg
{


template<class ColorT, class Order>
void comp_op_rgba_hue<ColorT,Order>::blend_pix(value_type* p,
                                               unsigned sr, unsigned sg, unsigned sb,
                                               unsigned sa, unsigned cover)
{
    if (cover < 255)
    {
        sr = (sr * cover + 255) >> 8;
        sg = (sg * cover + 255) >> 8;
        sb = (sb * cover + 255) >> 8;
        sa = (sa * cover + 255) >> 8;
    }

    if (sa > 0)
    {
        using namespace boost;
        using namespace gil;
        using namespace hsv_color_space;
        rgb8_pixel_t rgb_src(sr,sg,sb);
        rgb8_pixel_t rgb_dst(p[Order::R],p[Order::G],p[Order::B]);
        hsv32f_pixel_t hsv_src,hsv_dst;
        color_convert(rgb_src, hsv_src);
        color_convert(rgb_dst, hsv_dst);
        get_color(hsv_dst,hue_t()) = get_color(hsv_src,hue_t());
        color_convert(hsv_dst, rgb_dst);
        p[Order::R] = get_color(rgb_dst,red_t());
        p[Order::G] = get_color(rgb_dst,green_t());
        p[Order::B] = get_color(rgb_dst,blue_t());
        p[Order::A] = (value_type)(sa + p[Order::A] - ((sa * p[Order::A] + base_mask) >> base_shift));
    }
}

template<class ColorT, class Order>
void comp_op_rgba_saturation<ColorT,Order>::blend_pix(value_type* p,
                                                      unsigned sr, unsigned sg, unsigned sb,
                                                      unsigned sa, unsigned cover)
{
    if (cover < 255)
    {
        sr = (sr * cover + 255) >> 8;
        sg = (sg * cover + 255) >> 8;
        sb = (sb * cover + 255) >> 8;
        sa = (sa * cover + 255) >> 8;
    }

    if (sa > 0)
    {
        using namespace boost;
        using namespace gil;
        using namespace hsv_color_space;
        rgb8_pixel_t rgb_src(sr,sg,sb);
        rgb8_pixel_t rgb_dst(p[Order::R],p[Order::G],p[Order::B]);
        hsv32f_pixel_t hsv_src,hsv_dst;
        color_convert( rgb_src, hsv_src);
        color_convert( rgb_dst, hsv_dst);
        get_color(hsv_dst,saturation_t()) = get_color(hsv_src,saturation_t());
        color_convert(hsv_dst, rgb_dst);
        p[Order::R] = get_color(rgb_dst,red_t());
        p[Order::G] = get_color(rgb_dst,green_t());
        p[Order::B] = get_color(rgb_dst,blue_t());
        p[Order::A] = (value_type)(sa + p[Order::A] - ((sa * p[Order::A] + base_mask) >> base_shift));
    }
}

template<class ColorT, class Order>
void comp_op_rgba_color<ColorT,Order>::blend_pix(value_type* p,
                                                 unsigned sr, unsigned sg, unsigned sb,
                                                 unsigned sa, unsigned cover)
{
    if (cover < 255)
    {
        sr = (sr * cover + 255) >> 8;
        sg = (sg * cover + 255) >> 8;
        sb = (sb * cover + 255) >> 8;
        sa = (sa * cover + 255) >> 8;
    }

    if (sa > 0)
    {
        using namespace boost;
        using namespace gil;
        using namespace hsl_color_space;
        rgb8_pixel_t rgb_src(sr,sg,sb);
        rgb8_pixel_t rgb_dst(p[Order::R],p[Order::G],p[Order::B]);
        hsl32f_pixel_t hsl_src,hsl_dst;
        color_convert( rgb_src, hsl_src);
        color_convert( rgb_dst, hsl_dst);
        get_color(hsl_dst,hue_t()) = get_color(hsl_src,hue_t());
        get_color(hsl_dst,saturation_t()) = get_color(hsl_src,saturation_t());
        get_color(hsl_dst,lightness_t()) = get_color(hsl_dst,lightness_t());
        color_convert(hsl_dst, rgb_dst);
        p[Order::R] = get_color(rgb_dst,red_t());
        p[Order::G] = get_color(rgb_dst,green_t());
        p[Order::B] = get_color(rgb_dst,blue_t());
        p[Order::A] = (value_type)(sa + p[Order::A] - ((sa * p[Order::A] + base_mask) >> base_shift));
    }
}

template<class ColorT, class Order>
void comp_op_rgba_value<ColorT,Order>::blend_pix(value_type* p,
                                                 unsigned sr, unsigned sg, unsigned sb,
                                                 unsigned sa, unsigned cover)
{
    if (cover < 255)
    {
        sr = (sr * cover + 255) >> 8;
        sg = (sg * cover + 255) >> 8;
        sb = (sb * cover + 255) >> 8;
        sa = (sa * cover + 255) >> 8;
    }

    if (sa > 0)
    {
        using namespace boost;
        using namespace gil;
        using namespace hsv_color_space;
        rgb8_pixel_t rgb_src(sr,sg,sb);
        rgb8_pixel_t rgb_dst(p[Order::R],p[Order::G],p[Order::B]);
        hsv32f_pixel_t hsv_src,hsv_dst;
        color_convert( rgb_src, hsv_src);
        color_convert( rgb_dst, hsv_dst);
        get_color(hsv_dst,value_t()) = get_color(hsv_src,value_t());
        color_convert(hsv_dst, rgb_dst);
        p[Order::R] = get_color(rgb_dst,red_t());
        p[Order::G] = get_color(rgb_dst,green_t());
        p[Order::B] = get_color(rgb_dst,blue_t());
        p[Order::A] = (value_type)(sa + p[Order::A] - ((sa * p[Order::A] + base_mask) >> base_shift));
    }
}


template struct comp_op_rgba_hue<agg::rgba8, agg::order_rgba>;
template struct comp_op_rgba_saturation<agg::rgba8, agg::order_rgba>;
template struct comp_op_rgba_color<agg::rgba8, agg::order_rgba>;
template struct comp_op_rgba_value<agg::rgba8, agg::order_rgba>;



}
