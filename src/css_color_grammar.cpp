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
#include <boost/version.hpp>

#if BOOST_VERSION >= 104500

#include <mapnik/css_color_grammar.hpp>

namespace mapnik
{

named_colors_::named_colors_()
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
        ("grey", color(128, 128, 128))
        ("green", color(0, 128, 0))
        ("greenyellow", color(173, 255, 47))
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

double hue_to_rgb( double m1, double m2, double h)
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

template <typename Iterator>
css_color_grammar<Iterator>::css_color_grammar()
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

    hex_color = lit('#')
        >> hex2 [ at_c<0>(_val) = _1 ]
        >> hex2 [ at_c<1>(_val) = _1 ]
        >> hex2 [ at_c<2>(_val) = _1 ]
        >>-hex2 [ at_c<3>(_val) = _1 ]
        ;

    hex_color_small = lit('#')
        >> hex1 [ at_c<0>(_val) = _1 | _1 << 4 ]
        >> hex1 [ at_c<1>(_val) = _1 | _1 << 4 ]
        >> hex1 [ at_c<2>(_val) = _1 | _1 << 4 ]
        >>-hex1 [ at_c<3>(_val) = _1 | _1 << 4 ]
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

template struct mapnik::css_color_grammar<std::string::const_iterator>;


}

#endif