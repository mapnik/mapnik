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

#ifndef MAPNIK_CSS_COLOR_GRAMMAR_HPP
#define MAPNIK_CSS_COLOR_GRAMMAR_HPP

// mapnik
#include <mapnik/color.hpp>
#include <mapnik/util/hsl.hpp>
#include <mapnik/safe_cast.hpp>

// boost
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#pragma GCC diagnostic pop

// stl
#include <string>

namespace mapnik
{

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

using ascii_space_type = boost::spirit::ascii::space_type;

struct percent_conv_impl
{
    template <typename T>
    struct result
    {
        using type = unsigned;
    };

    unsigned operator() (double val) const
    {
        return safe_cast<uint8_t>(std::lround((255.0 * val)/100.0));
    }
};

struct alpha_conv_impl
{
    template <typename T>
    struct result
    {
        using type = unsigned;
    };

    unsigned operator() (double val) const
    {
        return safe_cast<uint8_t>(std::lround((255.0 * val)));
    }
};

struct hsl_conv_impl
{
    template<typename T>
    struct result
    {
        using type = void;
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

        c.set_red(safe_cast<uint8_t>(std::lround(255.0 * r)));
        c.set_green(safe_cast<uint8_t>(std::lround(255.0 * g)));
        c.set_blue(safe_cast<uint8_t>(std::lround(255.0 * b)));
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
    phoenix::function<percent_conv_impl> percent_converter;
    phoenix::function<alpha_conv_impl>   alpha_converter;
    phoenix::function<hsl_conv_impl>  hsl_converter;
};

}

#endif // MAPNIK_CSS_COLOR_GRAMMAR_HPP
