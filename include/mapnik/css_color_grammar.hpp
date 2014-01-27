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
#include <mapnik/util/hsl.hpp>

// spirit2
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_action.hpp>

// phoenix
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_function.hpp>

// stl
#include <string>

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
#ifdef BOOST_SPIRIT_USE_PHOENIX_V3
    template<typename T>
#else
    template<typename T0,typename T1, typename T2, typename T3>
#endif
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

#endif // MAPNIK_CSS_COLOR_GRAMMAR_HPP
