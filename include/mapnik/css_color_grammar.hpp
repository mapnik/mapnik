/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2009 Artem Pavlenko
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

//$Id$

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
// fusion
#include <boost/fusion/include/adapt_struct.hpp>

// not in boost 1.41 
//#include <boost/fusion/include/adapt_class.hpp>

// stl
#include <string>


//BOOST_FUSION_ADAPT_CLASS(
//    mapnik::color,
//   (unsigned, unsigned, obj.red(), obj.set_red(val))
//   (unsigned, unsigned, obj.green(), obj.set_green(val))
//   (unsigned, unsigned, obj.blue(), obj.set_blue(val))
//   (unsigned, unsigned, obj.alpha(), obj.set_alpha(val))
//   )

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

struct named_colors_ : qi::symbols<char,mapnik::css>
{
    named_colors_()
    {
        add
            ("aliceblue", mapnik::css(240, 248, 255))
            ("antiquewhite", mapnik::css(250, 235, 215))
            ("aqua", mapnik::css(0, 255, 255))
            ("aquamarine", mapnik::css(127, 255, 212))
            ("azure", mapnik::css(240, 255, 255))
            ("beige", mapnik::css(245, 245, 220))
            ("bisque", mapnik::css(255, 228, 196))
            ("black", mapnik::css(0, 0, 0))
            ("blanchedalmond", mapnik::css(255,235,205))
            ("blue", mapnik::css(0, 0, 255))
            ("blueviolet", mapnik::css(138, 43, 226))
            ("brown", mapnik::css(165, 42, 42))
            ("burlywood", mapnik::css(222, 184, 135))
            ("cadetblue", mapnik::css(95, 158, 160))
            ("chartreuse", mapnik::css(127, 255, 0))
            ("chocolate", mapnik::css(210, 105, 30))
            ("coral", mapnik::css(255, 127, 80))
            ("cornflowerblue", mapnik::css(100, 149, 237))
            ("cornsilk", mapnik::css(255, 248, 220))
            ("crimson", mapnik::css(220, 20, 60))
            ("cyan", mapnik::css(0, 255, 255))
            ("darkblue", mapnik::css(0, 0, 139))
            ("darkcyan", mapnik::css(0, 139, 139))
            ("darkgoldenrod", mapnik::css(184, 134, 11))
            ("darkgray", mapnik::css(169, 169, 169))
            ("darkgreen", mapnik::css(0, 100, 0))
            ("darkgrey", mapnik::css(169, 169, 169))
            ("darkkhaki", mapnik::css(189, 183, 107))
            ("darkmagenta", mapnik::css(139, 0, 139))
            ("darkolivegreen", mapnik::css(85, 107, 47))
            ("darkorange", mapnik::css(255, 140, 0))
            ("darkorchid", mapnik::css(153, 50, 204))
            ("darkred", mapnik::css(139, 0, 0))
            ("darksalmon", mapnik::css(233, 150, 122))
            ("darkseagreen", mapnik::css(143, 188, 143))
            ("darkslateblue", mapnik::css(72, 61, 139))
            ("darkslategrey", mapnik::css(47, 79, 79))
            ("darkturquoise", mapnik::css(0, 206, 209))
            ("darkviolet", mapnik::css(148, 0, 211))
            ("deeppink", mapnik::css(255, 20, 147))
            ("deepskyblue", mapnik::css(0, 191, 255))
            ("dimgray", mapnik::css(105, 105, 105))
            ("dimgrey", mapnik::css(105, 105, 105))
            ("dodgerblue", mapnik::css(30, 144, 255))
            ("firebrick", mapnik::css(178, 34, 34))
            ("floralwhite", mapnik::css(255, 250, 240))
            ("forestgreen", mapnik::css(34, 139, 34))
            ("fuchsia", mapnik::css(255, 0, 255))
            ("gainsboro", mapnik::css(220, 220, 220))
            ("ghostwhite", mapnik::css(248, 248, 255))
            ("gold", mapnik::css(255, 215, 0))
            ("goldenrod", mapnik::css(218, 165, 32))
            ("gray", mapnik::css(128, 128, 128))
            ("grey", mapnik::css(128, 128, 128))
            ("green", mapnik::css(0, 128, 0))
            ("greenyellow", mapnik::css(173, 255, 47))
            ("honeydew", mapnik::css(240, 255, 240))
            ("hotpink", mapnik::css(255, 105, 180))
            ("indianred", mapnik::css(205, 92, 92))
            ("indigo", mapnik::css(75, 0, 130))
            ("ivory", mapnik::css(255, 255, 240))
            ("khaki", mapnik::css(240, 230, 140))
            ("lavender", mapnik::css(230, 230, 250))
            ("lavenderblush", mapnik::css(255, 240, 245))
            ("lawngreen", mapnik::css(124, 252, 0))
            ("lemonchiffon", mapnik::css(255, 250, 205))
            ("lightblue", mapnik::css(173, 216, 230))
            ("lightcoral", mapnik::css(240, 128, 128))
            ("lightcyan", mapnik::css(224, 255, 255))
            ("lightgoldenrodyellow", mapnik::css(250, 250, 210))
            ("lightgray", mapnik::css(211, 211, 211))
            ("lightgreen", mapnik::css(144, 238, 144))
            ("lightgrey", mapnik::css(211, 211, 211))
            ("lightpink", mapnik::css(255, 182, 193))
            ("lightsalmon", mapnik::css(255, 160, 122))
            ("lightseagreen", mapnik::css(32, 178, 170))
            ("lightskyblue", mapnik::css(135, 206, 250))
            ("lightslategray", mapnik::css(119, 136, 153))
            ("lightslategrey", mapnik::css(119, 136, 153))
            ("lightsteelblue", mapnik::css(176, 196, 222))
            ("lightyellow", mapnik::css(255, 255, 224))
            ("lime", mapnik::css(0, 255, 0))
            ("limegreen", mapnik::css(50, 205, 50))
            ("linen", mapnik::css(250, 240, 230))
            ("magenta", mapnik::css(255, 0, 255))
            ("maroon", mapnik::css(128, 0, 0))
            ("mediumaquamarine", mapnik::css(102, 205, 170))
            ("mediumblue", mapnik::css(0, 0, 205))
            ("mediumorchid", mapnik::css(186, 85, 211))
            ("mediumpurple", mapnik::css(147, 112, 219))
            ("mediumseagreen", mapnik::css(60, 179, 113))
            ("mediumslateblue", mapnik::css(123, 104, 238))
            ("mediumspringgreen", mapnik::css(0, 250, 154))
            ("mediumturquoise", mapnik::css(72, 209, 204))
            ("mediumvioletred", mapnik::css(199, 21, 133))
            ("midnightblue", mapnik::css(25, 25, 112))
            ("mintcream", mapnik::css(245, 255, 250))
            ("mistyrose", mapnik::css(255, 228, 225))
            ("moccasin", mapnik::css(255, 228, 181))
            ("navajowhite", mapnik::css(255, 222, 173))
            ("navy", mapnik::css(0, 0, 128))
            ("oldlace", mapnik::css(253, 245, 230))
            ("olive", mapnik::css(128, 128, 0))
            ("olivedrab", mapnik::css(107, 142, 35))
            ("orange", mapnik::css(255, 165, 0))
            ("orangered", mapnik::css(255, 69, 0))
            ("orchid", mapnik::css(218, 112, 214))
            ("palegoldenrod", mapnik::css(238, 232, 170))
            ("palegreen", mapnik::css(152, 251, 152))
            ("paleturquoise", mapnik::css(175, 238, 238))
            ("palevioletred", mapnik::css(219, 112, 147))
            ("papayawhip", mapnik::css(255, 239, 213))
            ("peachpuff", mapnik::css(255, 218, 185))
            ("peru", mapnik::css(205, 133, 63))
            ("pink", mapnik::css(255, 192, 203))
            ("plum", mapnik::css(221, 160, 221))
            ("powderblue", mapnik::css(176, 224, 230))
            ("purple", mapnik::css(128, 0, 128))
            ("red", mapnik::css(255, 0, 0))
            ("rosybrown", mapnik::css(188, 143, 143))
            ("royalblue", mapnik::css(65, 105, 225))
            ("saddlebrown", mapnik::css(139, 69, 19))
            ("salmon", mapnik::css(250, 128, 114))
            ("sandybrown", mapnik::css(244, 164, 96))
            ("seagreen", mapnik::css(46, 139, 87))
            ("seashell", mapnik::css(255, 245, 238))
            ("sienna", mapnik::css(160, 82, 45))
            ("silver", mapnik::css(192, 192, 192))
            ("skyblue", mapnik::css(135, 206, 235))
            ("slateblue", mapnik::css(106, 90, 205))
            ("slategray", mapnik::css(112, 128, 144))
            ("slategrey", mapnik::css(112, 128, 144))
            ("snow", mapnik::css(255, 250, 250))
            ("springgreen", mapnik::css(0, 255, 127))
            ("steelblue", mapnik::css(70, 130, 180))
            ("tan", mapnik::css(210, 180, 140))
            ("teal", mapnik::css(0, 128, 128))
            ("thistle", mapnik::css(216, 191, 216))
            ("tomato", mapnik::css(255, 99, 71))
            ("turquoise", mapnik::css(64, 224, 208))
            ("violet", mapnik::css(238, 130, 238))
            ("wheat", mapnik::css(245, 222, 179))
            ("white", mapnik::css(255, 255, 255))
            ("whitesmoke", mapnik::css(245, 245, 245))
            ("yellow", mapnik::css(255, 255, 0))
            ("yellowgreen", mapnik::css(154, 205, 50))
            ("transparent", mapnik::css(0, 0, 0, 0))
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


template <typename Iterator>
struct css_color_grammar : qi::grammar<Iterator, mapnik::css(), ascii_space_type>
{
    
    css_color_grammar()
        : css_color_grammar::base_type(css_color)
          
    {
        using qi::lit;
        using qi::_val;
        using qi::double_;
        using qi::_1;
        using ascii::no_case;        
        using phoenix::at_c;
        
        css_color %= rgba_color
            | rgba_percent_color 
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
    }
    
    qi::uint_parser< unsigned, 16, 2, 2 > hex2 ;
    qi::uint_parser< unsigned, 16, 1, 1 > hex1 ;
    qi::uint_parser< unsigned, 10, 1, 3 > dec3 ;
    qi::rule<Iterator, mapnik::css(), ascii_space_type> rgba_color;
    qi::rule<Iterator, mapnik::css(), ascii_space_type> rgba_percent_color;
    qi::rule<Iterator, mapnik::css(), ascii_space_type> hex_color;
    qi::rule<Iterator, mapnik::css(), ascii_space_type> hex_color_small;
    qi::rule<Iterator, mapnik::css(), ascii_space_type> css_color;
    named_colors_ named;
    phoenix::function<percent_conv_impl> percent_converter;
    phoenix::function<alpha_conv_impl>   alpha_converter; 
};

}

#endif //MAPNIK_CSS_COLOR_GRAMMAR_HPP 
