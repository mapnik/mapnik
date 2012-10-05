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

#ifndef MAPNIK_CSS_COLOR_GRAMMAR_HPP
#define MAPNIK_CSS_COLOR_GRAMMAR_HPP

// mapnik
#include <mapnik/color.hpp>

// spirit2
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_action.hpp>

// phoenix
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_function.hpp>

// stl
#include <string>

namespace mapnik {

// http://www.w3.org/TR/css3-color/#hsl-color
inline double hue_to_rgb( double m1, double m2, double h)
{
    if (h < 0.0) h = h + 1.0;
    else if (h > 1) h = h - 1.0;

    if (h * 6 < 1.0)
        return m1 + (m2 - m1) * h * 6.0;
    if (h * 2 < 1.0)
        return m2;
    if (h * 3 < 2.0)
        return m1 + (m2 - m1)* (2.0/3.0 - h) * 6.0;
    return m1;
}

} // namespace mapnik

// boost
#include <boost/version.hpp>
#if BOOST_VERSION >= 104500
// fusion
#include <boost/fusion/include/adapt_adt.hpp>

BOOST_FUSION_ADAPT_ADT(
    mapnik::color,
    (unsigned, unsigned, obj.red(), obj.set_red(val))
    (unsigned, unsigned, obj.green(), obj.set_green(val))
    (unsigned, unsigned, obj.blue(), obj.set_blue(val))
    (unsigned, unsigned, obj.alpha(), obj.set_alpha(val))
    )

namespace mapnik
{

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

typedef boost::spirit::ascii::space_type ascii_space_type;

struct named_colors_ : qi::symbols<char,color>
{
    named_colors_();
} ;

// clipper helper
template <int MIN,int MAX>
inline int clip_int(int val)
{
    if (val < MIN ) return MIN;
    if (val > MAX ) return MAX;
    return val;
}

struct percent_conv_impl
{
    template <typename T>
    struct result
    {
        typedef unsigned type;
    };

    unsigned operator() (double val) const
    {
        return clip_int<0,255>(int((255.0 * val)/100.0 + 0.5));
    }
};

struct alpha_conv_impl
{
    template <typename T>
    struct result
    {
        typedef unsigned type;
    };

    unsigned operator() (double val) const
    {
        return clip_int<0,255>(int((255.0 * val) + 0.5));
    }
};

struct hsl_conv_impl
{
    template<typename T0,typename T1, typename T2, typename T3>
    struct result
    {
        typedef void type;
    };

    template <typename T0,typename T1, typename T2, typename T3>
    void operator() (T0 & c, T1 h, T2 s, T3 l) const
    {
        double m1,m2;
        // normalize values
        h /= 360.0;
        s /= 100.0;
        l /= 100.0;

        if (l <= 0.5)
            m2 = l * (s + 1.0);
        else
            m2 = l + s - l*s;
        m1 = l * 2 - m2;

        double r = hue_to_rgb(m1, m2, h + 1.0/3.0);
        double g = hue_to_rgb(m1, m2, h);
        double b = hue_to_rgb(m1, m2, h - 1.0/3.0);

        c.set_red(clip_int<0,255>(int((255.0 * r) + 0.5)));
        c.set_green(clip_int<0,255>(int((255.0 * g) + 0.5)));
        c.set_blue(clip_int<0,255>(int((255.0 * b) + 0.5)));
    }
};


template <typename Iterator>
struct css_color_grammar : qi::grammar<Iterator, color(), ascii_space_type>
{
    css_color_grammar();
    qi::uint_parser< unsigned, 16, 2, 2 > hex2 ;
    qi::uint_parser< unsigned, 16, 1, 1 > hex1 ;
    qi::uint_parser< unsigned, 10, 1, 3 > dec3 ;
    qi::rule<Iterator, color(), ascii_space_type> rgba_color;
    qi::rule<Iterator, color(), ascii_space_type> rgba_percent_color;
    qi::rule<Iterator, qi::locals<double,double,double>,color(), ascii_space_type> hsl_percent_color;
    qi::rule<Iterator, color(), ascii_space_type> hex_color;
    qi::rule<Iterator, color(), ascii_space_type> hex_color_small;
    qi::rule<Iterator, color(), ascii_space_type> css_color;
    named_colors_ named;
    phoenix::function<percent_conv_impl> percent_converter;
    phoenix::function<alpha_conv_impl>   alpha_converter;
    phoenix::function<hsl_conv_impl>  hsl_converter;
};

}

#else // boost 1.42 compatibility

#include <boost/fusion/include/adapt_struct.hpp>

namespace mapnik
{
// temp workaround . TODO: adapt mapnik::color
struct css
{
    css ()
        : r(255),g(255),b(255),a(255) {}
    css(unsigned r_,unsigned g_, unsigned b_,unsigned a_ = 0xff)
        : r(r_),g(g_),b(b_),a(a_) {}

    unsigned r;
    unsigned g;
    unsigned b;
    unsigned a;
};

}

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::css,
    (unsigned, r)
    (unsigned, g)
    (unsigned, b)
    (unsigned, a)
    )

namespace mapnik
{

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

typedef boost::spirit::ascii::space_type ascii_space_type;

struct named_colors_ : qi::symbols<char,css>
{
    named_colors_()
    {
        add
            ("aliceblue", css(240, 248, 255))
            ("antiquewhite", css(250, 235, 215))
            ("aqua", css(0, 255, 255))
            ("aquamarine", css(127, 255, 212))
            ("azure", css(240, 255, 255))
            ("beige", css(245, 245, 220))
            ("bisque", css(255, 228, 196))
            ("black", css(0, 0, 0))
            ("blanchedalmond", css(255,235,205))
            ("blue", css(0, 0, 255))
            ("blueviolet", css(138, 43, 226))
            ("brown", css(165, 42, 42))
            ("burlywood", css(222, 184, 135))
            ("cadetblue", css(95, 158, 160))
            ("chartreuse", css(127, 255, 0))
            ("chocolate", css(210, 105, 30))
            ("coral", css(255, 127, 80))
            ("cornflowerblue", css(100, 149, 237))
            ("cornsilk", css(255, 248, 220))
            ("crimson", css(220, 20, 60))
            ("cyan", css(0, 255, 255))
            ("darkblue", css(0, 0, 139))
            ("darkcyan", css(0, 139, 139))
            ("darkgoldenrod", css(184, 134, 11))
            ("darkgray", css(169, 169, 169))
            ("darkgreen", css(0, 100, 0))
            ("darkgrey", css(169, 169, 169))
            ("darkkhaki", css(189, 183, 107))
            ("darkmagenta", css(139, 0, 139))
            ("darkolivegreen", css(85, 107, 47))
            ("darkorange", css(255, 140, 0))
            ("darkorchid", css(153, 50, 204))
            ("darkred", css(139, 0, 0))
            ("darksalmon", css(233, 150, 122))
            ("darkseagreen", css(143, 188, 143))
            ("darkslateblue", css(72, 61, 139))
            ("darkslategrey", css(47, 79, 79))
            ("darkturquoise", css(0, 206, 209))
            ("darkviolet", css(148, 0, 211))
            ("deeppink", css(255, 20, 147))
            ("deepskyblue", css(0, 191, 255))
            ("dimgray", css(105, 105, 105))
            ("dimgrey", css(105, 105, 105))
            ("dodgerblue", css(30, 144, 255))
            ("firebrick", css(178, 34, 34))
            ("floralwhite", css(255, 250, 240))
            ("forestgreen", css(34, 139, 34))
            ("fuchsia", css(255, 0, 255))
            ("gainsboro", css(220, 220, 220))
            ("ghostwhite", css(248, 248, 255))
            ("gold", css(255, 215, 0))
            ("goldenrod", css(218, 165, 32))
            ("gray", css(128, 128, 128))
            ("grey", css(128, 128, 128))
            ("green", css(0, 128, 0))
            ("greenyellow", css(173, 255, 47))
            ("honeydew", css(240, 255, 240))
            ("hotpink", css(255, 105, 180))
            ("indianred", css(205, 92, 92))
            ("indigo", css(75, 0, 130))
            ("ivory", css(255, 255, 240))
            ("khaki", css(240, 230, 140))
            ("lavender", css(230, 230, 250))
            ("lavenderblush", css(255, 240, 245))
            ("lawngreen", css(124, 252, 0))
            ("lemonchiffon", css(255, 250, 205))
            ("lightblue", css(173, 216, 230))
            ("lightcoral", css(240, 128, 128))
            ("lightcyan", css(224, 255, 255))
            ("lightgoldenrodyellow", css(250, 250, 210))
            ("lightgray", css(211, 211, 211))
            ("lightgreen", css(144, 238, 144))
            ("lightgrey", css(211, 211, 211))
            ("lightpink", css(255, 182, 193))
            ("lightsalmon", css(255, 160, 122))
            ("lightseagreen", css(32, 178, 170))
            ("lightskyblue", css(135, 206, 250))
            ("lightslategray", css(119, 136, 153))
            ("lightslategrey", css(119, 136, 153))
            ("lightsteelblue", css(176, 196, 222))
            ("lightyellow", css(255, 255, 224))
            ("lime", css(0, 255, 0))
            ("limegreen", css(50, 205, 50))
            ("linen", css(250, 240, 230))
            ("magenta", css(255, 0, 255))
            ("maroon", css(128, 0, 0))
            ("mediumaquamarine", css(102, 205, 170))
            ("mediumblue", css(0, 0, 205))
            ("mediumorchid", css(186, 85, 211))
            ("mediumpurple", css(147, 112, 219))
            ("mediumseagreen", css(60, 179, 113))
            ("mediumslateblue", css(123, 104, 238))
            ("mediumspringgreen", css(0, 250, 154))
            ("mediumturquoise", css(72, 209, 204))
            ("mediumvioletred", css(199, 21, 133))
            ("midnightblue", css(25, 25, 112))
            ("mintcream", css(245, 255, 250))
            ("mistyrose", css(255, 228, 225))
            ("moccasin", css(255, 228, 181))
            ("navajowhite", css(255, 222, 173))
            ("navy", css(0, 0, 128))
            ("oldlace", css(253, 245, 230))
            ("olive", css(128, 128, 0))
            ("olivedrab", css(107, 142, 35))
            ("orange", css(255, 165, 0))
            ("orangered", css(255, 69, 0))
            ("orchid", css(218, 112, 214))
            ("palegoldenrod", css(238, 232, 170))
            ("palegreen", css(152, 251, 152))
            ("paleturquoise", css(175, 238, 238))
            ("palevioletred", css(219, 112, 147))
            ("papayawhip", css(255, 239, 213))
            ("peachpuff", css(255, 218, 185))
            ("peru", css(205, 133, 63))
            ("pink", css(255, 192, 203))
            ("plum", css(221, 160, 221))
            ("powderblue", css(176, 224, 230))
            ("purple", css(128, 0, 128))
            ("red", css(255, 0, 0))
            ("rosybrown", css(188, 143, 143))
            ("royalblue", css(65, 105, 225))
            ("saddlebrown", css(139, 69, 19))
            ("salmon", css(250, 128, 114))
            ("sandybrown", css(244, 164, 96))
            ("seagreen", css(46, 139, 87))
            ("seashell", css(255, 245, 238))
            ("sienna", css(160, 82, 45))
            ("silver", css(192, 192, 192))
            ("skyblue", css(135, 206, 235))
            ("slateblue", css(106, 90, 205))
            ("slategray", css(112, 128, 144))
            ("slategrey", css(112, 128, 144))
            ("snow", css(255, 250, 250))
            ("springgreen", css(0, 255, 127))
            ("steelblue", css(70, 130, 180))
            ("tan", css(210, 180, 140))
            ("teal", css(0, 128, 128))
            ("thistle", css(216, 191, 216))
            ("tomato", css(255, 99, 71))
            ("turquoise", css(64, 224, 208))
            ("violet", css(238, 130, 238))
            ("wheat", css(245, 222, 179))
            ("white", css(255, 255, 255))
            ("whitesmoke", css(245, 245, 245))
            ("yellow", css(255, 255, 0))
            ("yellowgreen", css(154, 205, 50))
            ("transparent", css(0, 0, 0, 0))
            ;
    }
} ;

// clipper helper

template <int MIN,int MAX>
inline int clip_int(int val)
{
    if (val < MIN ) return MIN;
    if (val > MAX ) return MAX;
    return val;
}

struct percent_conv_impl
{
    template <typename T>
    struct result
    {
        typedef unsigned type;
    };

    unsigned operator() (double val) const
    {
        return clip_int<0,255>(int((255.0 * val)/100.0 + 0.5));
    }
};

struct alpha_conv_impl
{
    template <typename T>
    struct result
    {
        typedef unsigned type;
    };

    unsigned operator() (double val) const
    {
        return clip_int<0,255>(int((255.0 * val) + 0.5));
    }
};

struct hsl_conv_impl
{
    template<typename T0,typename T1, typename T2, typename T3>
    struct result
    {
        typedef void type;
    };

    template <typename T0,typename T1, typename T2, typename T3>
    void operator() (T0 & c, T1 h, T2 s, T3 l) const
    {
        double m1,m2;
        // normalize values
        h /= 360.0;
        s /= 100.0;
        l /= 100.0;

        if (l <= 0.5)
            m2 = l * (s + 1.0);
        else
            m2 = l + s - l*s;
        m1 = l * 2 - m2;

        double r = hue_to_rgb(m1, m2, h + 1.0/3.0);
        double g = hue_to_rgb(m1, m2, h);
        double b = hue_to_rgb(m1, m2, h - 1.0/3.0);

        c.r = clip_int<0,255>(int((255.0 * r) + 0.5));
        c.g = clip_int<0,255>(int((255.0 * g) + 0.5));
        c.b = clip_int<0,255>(int((255.0 * b) + 0.5));
    }
};


template <typename Iterator>
struct css_color_grammar : qi::grammar<Iterator, css(), ascii_space_type>
{

    css_color_grammar()
        : css_color_grammar::base_type(css_color)

    {
        using qi::lit;
        using qi::_val;
        using qi::double_;
        using qi::_1;
        using qi::_a;
        using qi::_b;
        using qi::_c;
        using ascii::no_case;
        using phoenix::at_c;

        css_color %= rgba_color
            | rgba_percent_color
            | hsl_percent_color
            | hex_color
            | hex_color_small
            | no_case[named];

        hex_color %= lit('#')
            >> hex2
            >> hex2
            >> hex2
            >> -hex2
            ;

        hex_color_small = lit('#')
            >> hex1 [ at_c<0>(_val) = _1 | _1 << 4 ]
            >> hex1 [ at_c<1>(_val) = _1 | _1 << 4 ]
            >> hex1 [ at_c<2>(_val) = _1 | _1 << 4 ]
            >> -hex1[ at_c<3>(_val) = _1 | _1 << 4 ]
            ;

        rgba_color = lit("rgb") >> -lit('a')
                                >> lit('(')
                                >> dec3 [at_c<0>(_val) = _1] >> ','
                                >> dec3 [at_c<1>(_val) = _1] >> ','
                                >> dec3 [at_c<2>(_val) = _1]
                                >> -(','>> -double_ [at_c<3>(_val) = alpha_converter(_1)])
                                >> lit(')')
            ;

        rgba_percent_color = lit("rgb") >> -lit('a')
                                        >> lit('(')
                                        >> double_ [at_c<0>(_val) = percent_converter(_1)] >> '%' >> ','
                                        >> double_ [at_c<1>(_val) = percent_converter(_1)] >> '%' >> ','
                                        >> double_ [at_c<2>(_val) = percent_converter(_1)] >> '%'
                                        >> -(','>> -double_ [at_c<3>(_val) = alpha_converter(_1)])
                                        >> lit(')')
            ;

        hsl_percent_color = lit("hsl") >> -lit('a')
                                       >> lit('(')
                                       >> double_ [ _a = _1] >> ','        // hue 0..360
                                       >> double_ [ _b = _1] >> '%' >> ',' // saturation 0..100%
                                       >> double_ [ _c = _1] >> '%'        // lightness  0..100%
                                       >> -(','>> -double_ [at_c<3>(_val) = alpha_converter(_1)]) // opacity 0...1
                                       >> lit (')') [ hsl_converter(_val,_a,_b,_c)]
            ;
    }

    qi::uint_parser< unsigned, 16, 2, 2 > hex2 ;
    qi::uint_parser< unsigned, 16, 1, 1 > hex1 ;
    qi::uint_parser< unsigned, 10, 1, 3 > dec3 ;
    qi::rule<Iterator, css(), ascii_space_type> rgba_color;
    qi::rule<Iterator, css(), ascii_space_type> rgba_percent_color;
    qi::rule<Iterator, qi::locals<double,double,double>,css(), ascii_space_type> hsl_percent_color;
    qi::rule<Iterator, css(), ascii_space_type> hex_color;
    qi::rule<Iterator, css(), ascii_space_type> hex_color_small;
    qi::rule<Iterator, css(), ascii_space_type> css_color;
    named_colors_ named;
    phoenix::function<percent_conv_impl> percent_converter;
    phoenix::function<alpha_conv_impl>   alpha_converter;
    phoenix::function<hsl_conv_impl>  hsl_converter;
};

}

#endif

#endif // MAPNIK_CSS_COLOR_GRAMMAR_HPP
