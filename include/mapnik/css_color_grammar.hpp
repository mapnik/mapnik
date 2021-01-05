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
#pragma GCC diagnostic pop

// stl
#include <string>

namespace mapnik
{

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

using ascii_space_type = boost::spirit::ascii::space_type;

template <typename Iterator>
struct css_color_grammar : qi::grammar<Iterator, color(), ascii_space_type>
{
    // ctor
    css_color_grammar();
    // rules
    qi::uint_parser< unsigned, 16, 2, 2 > hex2 ;
    qi::uint_parser< unsigned, 16, 1, 1 > hex1 ;
    qi::uint_parser< unsigned, 10, 1, 3 > dec3 ;
    qi::rule<Iterator, color(), ascii_space_type> rgba_color;
    qi::rule<Iterator, color(), ascii_space_type> rgba_percent_color;
    qi::rule<Iterator, qi::locals<double,double,double>,color(), ascii_space_type> hsl_percent_color;
    qi::rule<Iterator, color(), ascii_space_type> hex_color;
    qi::rule<Iterator, color(), ascii_space_type> hex_color_small;
    qi::rule<Iterator, color(), ascii_space_type> css_color;
};

}

#endif // MAPNIK_CSS_COLOR_GRAMMAR_HPP
