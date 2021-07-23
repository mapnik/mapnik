/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

// REF: http://www.w3.org/TR/css3-color/

#ifndef MAPNIK_CSS_COLOR_GRAMMAR_X3_DEF_HPP
#define MAPNIK_CSS_COLOR_GRAMMAR_X3_DEF_HPP

#include <mapnik/css/css_color_grammar_x3.hpp>
#include <mapnik/util/hsl.hpp>
#include <mapnik/safe_cast.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/adapted/struct.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
MAPNIK_DISABLE_WARNING_POP

BOOST_FUSION_ADAPT_STRUCT (
    mapnik::color,
    (std::uint8_t, red_)
    (std::uint8_t, green_)
    (std::uint8_t, blue_)
    (std::uint8_t, alpha_)
    )

namespace mapnik {

namespace x3 = boost::spirit::x3;

namespace css_color_grammar {

using x3::lit;
using x3::uint_parser;
using x3::hex;
using x3::symbols;
using x3::omit;
using x3::attr;
using x3::double_;
using x3::no_case;
using x3::no_skip;

struct named_colors_ : x3::symbols<color>
{
    named_colors_()
    {
        add
            ("aliceblue", color(240, 248, 255))
            ("antiquewhite", color(250, 235, 215))
            ("aqua", color(0, 255, 255))
            ("aquamarine", color(127, 255, 212))
            ("azure", color(240, 255, 255))
            ("beige", color(245, 245, 220))
            ("bisque", color(255, 228, 196))
            ("black", color(0, 0, 0))
            ("blanchedalmond", color(255,235,205))
            ("blue", color(0, 0, 255))
            ("blueviolet", color(138, 43, 226))
            ("brown", color(165, 42, 42))
            ("burlywood", color(222, 184, 135))
            ("cadetblue", color(95, 158, 160))
            ("chartreuse", color(127, 255, 0))
            ("chocolate", color(210, 105, 30))
            ("coral", color(255, 127, 80))
            ("cornflowerblue", color(100, 149, 237))
            ("cornsilk", color(255, 248, 220))
            ("crimson", color(220, 20, 60))
            ("cyan", color(0, 255, 255))
            ("darkblue", color(0, 0, 139))
            ("darkcyan", color(0, 139, 139))
            ("darkgoldenrod", color(184, 134, 11))
            ("darkgray", color(169, 169, 169))
            ("darkgreen", color(0, 100, 0))
            ("darkgrey", color(169, 169, 169))
            ("darkkhaki", color(189, 183, 107))
            ("darkmagenta", color(139, 0, 139))
            ("darkolivegreen", color(85, 107, 47))
            ("darkorange", color(255, 140, 0))
            ("darkorchid", color(153, 50, 204))
            ("darkred", color(139, 0, 0))
            ("darksalmon", color(233, 150, 122))
            ("darkseagreen", color(143, 188, 143))
            ("darkslateblue", color(72, 61, 139))
            ("darkslategray", color(47, 79, 79))
            ("darkslategrey", color(47, 79, 79))
            ("darkturquoise", color(0, 206, 209))
            ("darkviolet", color(148, 0, 211))
            ("deeppink", color(255, 20, 147))
            ("deepskyblue", color(0, 191, 255))
            ("dimgray", color(105, 105, 105))
            ("dimgrey", color(105, 105, 105))
            ("dodgerblue", color(30, 144, 255))
            ("firebrick", color(178, 34, 34))
            ("floralwhite", color(255, 250, 240))
            ("forestgreen", color(34, 139, 34))
            ("fuchsia", color(255, 0, 255))
            ("gainsboro", color(220, 220, 220))
            ("ghostwhite", color(248, 248, 255))
            ("gold", color(255, 215, 0))
            ("goldenrod", color(218, 165, 32))
            ("gray", color(128, 128, 128))
            ("green", color(0, 128, 0))
            ("greenyellow", color(173, 255, 47))
            ("grey", color(128, 128, 128))
            ("honeydew", color(240, 255, 240))
            ("hotpink", color(255, 105, 180))
            ("indianred", color(205, 92, 92))
            ("indigo", color(75, 0, 130))
            ("ivory", color(255, 255, 240))
            ("khaki", color(240, 230, 140))
            ("lavender", color(230, 230, 250))
            ("lavenderblush", color(255, 240, 245))
            ("lawngreen", color(124, 252, 0))
            ("lemonchiffon", color(255, 250, 205))
            ("lightblue", color(173, 216, 230))
            ("lightcoral", color(240, 128, 128))
            ("lightcyan", color(224, 255, 255))
            ("lightgoldenrodyellow", color(250, 250, 210))
            ("lightgray", color(211, 211, 211))
            ("lightgreen", color(144, 238, 144))
            ("lightgrey", color(211, 211, 211))
            ("lightpink", color(255, 182, 193))
            ("lightsalmon", color(255, 160, 122))
            ("lightseagreen", color(32, 178, 170))
            ("lightskyblue", color(135, 206, 250))
            ("lightslategray", color(119, 136, 153))
            ("lightslategrey", color(119, 136, 153))
            ("lightsteelblue", color(176, 196, 222))
            ("lightyellow", color(255, 255, 224))
            ("lime", color(0, 255, 0))
            ("limegreen", color(50, 205, 50))
            ("linen", color(250, 240, 230))
            ("magenta", color(255, 0, 255))
            ("maroon", color(128, 0, 0))
            ("mediumaquamarine", color(102, 205, 170))
            ("mediumblue", color(0, 0, 205))
            ("mediumorchid", color(186, 85, 211))
            ("mediumpurple", color(147, 112, 219))
            ("mediumseagreen", color(60, 179, 113))
            ("mediumslateblue", color(123, 104, 238))
            ("mediumspringgreen", color(0, 250, 154))
            ("mediumturquoise", color(72, 209, 204))
            ("mediumvioletred", color(199, 21, 133))
            ("midnightblue", color(25, 25, 112))
            ("mintcream", color(245, 255, 250))
            ("mistyrose", color(255, 228, 225))
            ("moccasin", color(255, 228, 181))
            ("navajowhite", color(255, 222, 173))
            ("navy", color(0, 0, 128))
            ("oldlace", color(253, 245, 230))
            ("olive", color(128, 128, 0))
            ("olivedrab", color(107, 142, 35))
            ("orange", color(255, 165, 0))
            ("orangered", color(255, 69, 0))
            ("orchid", color(218, 112, 214))
            ("palegoldenrod", color(238, 232, 170))
            ("palegreen", color(152, 251, 152))
            ("paleturquoise", color(175, 238, 238))
            ("palevioletred", color(219, 112, 147))
            ("papayawhip", color(255, 239, 213))
            ("peachpuff", color(255, 218, 185))
            ("peru", color(205, 133, 63))
            ("pink", color(255, 192, 203))
            ("plum", color(221, 160, 221))
            ("powderblue", color(176, 224, 230))
            ("purple", color(128, 0, 128))
            ("rebeccapurple", color(102, 51, 153))
            ("red", color(255, 0, 0))
            ("rosybrown", color(188, 143, 143))
            ("royalblue", color(65, 105, 225))
            ("saddlebrown", color(139, 69, 19))
            ("salmon", color(250, 128, 114))
            ("sandybrown", color(244, 164, 96))
            ("seagreen", color(46, 139, 87))
            ("seashell", color(255, 245, 238))
            ("sienna", color(160, 82, 45))
            ("silver", color(192, 192, 192))
            ("skyblue", color(135, 206, 235))
            ("slateblue", color(106, 90, 205))
            ("slategray", color(112, 128, 144))
            ("slategrey", color(112, 128, 144))
            ("snow", color(255, 250, 250))
            ("springgreen", color(0, 255, 127))
            ("steelblue", color(70, 130, 180))
            ("tan", color(210, 180, 140))
            ("teal", color(0, 128, 128))
            ("thistle", color(216, 191, 216))
            ("tomato", color(255, 99, 71))
            ("turquoise", color(64, 224, 208))
            ("violet", color(238, 130, 238))
            ("wheat", color(245, 222, 179))
            ("white", color(255, 255, 255))
            ("whitesmoke", color(245, 245, 245))
            ("yellow", color(255, 255, 0))
            ("yellowgreen", color(154, 205, 50))
            ("transparent", color(0, 0, 0, 0))
            ;
    }
} named_colors;

x3::uint_parser<std::uint8_t, 16, 2, 2> hex2;
x3::uint_parser<std::uint8_t, 16, 1, 1> hex1;
x3::uint_parser<std::uint16_t, 10, 1, 3> dec3;

// rules
x3::rule<class hex2_color, color> const hex2_color("hex2_color");
x3::rule<class hex1_color, color> const hex1_color("hex1_color");
x3::rule<class rgb_color,  color> const rgb_color("rgb_color");
x3::rule<class rgba_color, color> const rgba_color("rgba_color");
x3::rule<class rgb_color_percent, color> const rgb_color_percent("rgb_color_percent");
x3::rule<class rgba_color_percent, color> const rgba_color_percent("rgba_color_percent");

struct clip_opacity
{
    static double call(double val)
    {
        if (val > 1.0) return 1.0;
        if (val < 0.0) return 0.0;
        return val;
    }
};

struct percent_converter
{
    static std::uint8_t call(double val)
    {
        return safe_cast<std::uint8_t>(std::lround((255.0 * val)/100.0));
    }
};

auto dec_red = [](auto& ctx)
{
    _val(ctx).red_ = _attr(ctx);
};

auto dec_green = [](auto& ctx)
{
    _val(ctx).green_ = _attr(ctx);
};

auto dec_blue = [](auto& ctx)
{
    _val(ctx).blue_ = _attr(ctx);
};

auto opacity = [](auto& ctx)
{
    _val(ctx).alpha_ = uint8_t((255.0 * clip_opacity::call(_attr(ctx))) + 0.5);
};

auto percent_red = [] (auto & ctx)
{
    _val(ctx).red_ = percent_converter::call(_attr(ctx));
};

auto percent_green = [] (auto & ctx)
{
    _val(ctx).green_ = percent_converter::call(_attr(ctx));
};

auto percent_blue = [] (auto & ctx)
{
    _val(ctx).blue_ = percent_converter::call(_attr(ctx));
};

auto hex1_red = [](auto& ctx)
{
    _val(ctx).red_ = _attr(ctx) | _attr(ctx) << 4;
};

auto hex1_green = [](auto& ctx)
{
    _val(ctx).green_ = _attr(ctx) | _attr(ctx) << 4;
};

auto hex1_blue = [](auto& ctx)
{
    _val(ctx).blue_ = _attr(ctx) | _attr(ctx) << 4;
};

auto hex1_opacity = [](auto& ctx)
{
    _val(ctx).alpha_ = _attr(ctx) | _attr(ctx) << 4;
};

auto hex2_red = [](auto& ctx)
{
    _val(ctx).red_ = _attr(ctx);
};

auto hex2_green = [](auto& ctx)
{
    _val(ctx).green_ = _attr(ctx);
};

auto hex2_blue = [](auto& ctx)
{
    _val(ctx).blue_ = _attr(ctx);
};

auto hex2_opacity = [](auto& ctx)
{
    _val(ctx).alpha_ = _attr(ctx);
};

auto hsl_to_rgba = [] (auto& ctx)
{
    double h = std::get<0>(_attr(ctx));
    double s = std::get<1>(_attr(ctx));
    double l = std::get<2>(_attr(ctx));
    double m1;
    double m2;
    // normalise values
    h /= 360.0;
    s /= 100.0;
    l /= 100.0;
    if (l <= 0.5)
    {
        m2 = l * (s + 1.0);
    }
    else
    {
        m2 = l + s - l*s;
    }
    m1 = l * 2 - m2;

    double r = hue_to_rgb(m1, m2, h + 1.0/3.0);
    double g = hue_to_rgb(m1, m2, h);
    double b = hue_to_rgb(m1, m2, h - 1.0/3.0);
    uint8_t alpha = uint8_t((255.0 * clip_opacity::call(std::get<3>(_attr(ctx)))) + 0.5);
    _val(ctx) = color(safe_cast<uint8_t>(std::lround(255.0 * r)),
                      safe_cast<uint8_t>(std::lround(255.0 * g)),
                      safe_cast<uint8_t>(std::lround(255.0 * b)),
                      alpha);
};

auto const hex2_color_def = no_skip[lit('#')
                                    >> hex2[hex2_red]
                                    >> hex2[hex2_green]
                                    >> hex2[hex2_blue]
                                    >> (hex2[hex2_opacity] | attr(255)[hex2_opacity])];

auto const hex1_color_def = no_skip[lit('#')
                                    >> hex1[hex1_red]
                                    >> hex1[hex1_green]
                                    >> hex1[hex1_blue]
                                    >> (hex1[hex1_opacity] | attr(15)[hex1_opacity])];

auto const rgb_color_def = lit("rgb")
    >> lit('(') >> dec3[dec_red]
    >> lit(',') >> dec3[dec_green]
    >> lit(',') >> dec3[dec_blue]
    >> attr(255) >> lit(')');

auto const rgb_color_percent_def = lit("rgb")
    >> lit('(') >> dec3[percent_red] >> lit('%')
    >> lit(',') >> dec3[percent_green] >> lit('%')
    >> lit(',') >> dec3[percent_blue] >> lit('%')
    >> attr(255) >> lit(')');

auto const rgba_color_def = lit("rgba")
    >> lit('(') >> dec3[dec_red]
    >> lit(',') >> dec3[dec_green]
    >> lit(',') >> dec3[dec_blue]
    >> lit(',') >> double_[opacity] >> lit(')');

auto const rgba_color_percent_def = lit("rgba")
    >> lit('(') >> dec3[percent_red] >> lit('%')
    >> lit(',') >> dec3[percent_green] >> lit('%')
    >> lit(',') >> dec3[percent_blue] >> lit('%')
    >> lit(',') >> double_[opacity] >> lit(')');

auto const hsl_values = x3::rule<class hsl_values, std::tuple<std::uint16_t,std::uint8_t,std::uint8_t, double >> {} =
    lit("hsl")
    >> lit('(') >> dec3
    >> lit(',') >> dec3 >> lit('%')
    >> lit(',') >> dec3 >> lit('%')
    >> attr(1.0) >> lit(')')
    ;

auto const hsla_values = x3::rule<class hsla_values, std::tuple<std::uint16_t,std::uint8_t,std::uint8_t, double >> {} =
    lit("hsla")
    >> lit('(') >> dec3
    >> lit(',') >> dec3 >> lit('%')
    >> lit(',') >> dec3 >> lit('%')
    >> lit(',') >> double_ >> lit(')')
    ;

auto const hsl_color = x3::rule<class hsl_color, color> {} = hsl_values[hsl_to_rgba];
auto const hsla_color = x3::rule<class hsla_color, color> {} = hsla_values[hsl_to_rgba];

auto const css_color_def =
    no_case[named_colors]
    |
    hex2_color
    |
    hex1_color
    |
    rgb_color
    |
    rgba_color
    |
    rgb_color_percent
    |
    rgba_color_percent
    |
    hsl_color
    |
    hsla_color
    ;

MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
BOOST_SPIRIT_DEFINE(
    css_color,
    hex2_color,
    hex1_color,
    rgb_color,
    rgba_color,
    rgb_color_percent,
    rgba_color_percent
    );
MAPNIK_DISABLE_WARNING_POP

} // ns
} //ns mapnik

#endif //MAPNIK_CSS_COLOR_GRAMMAR_X3_DEF_HPP
