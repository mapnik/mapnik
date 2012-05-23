/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

#ifndef MAPNIK_IMAGE_FILITER_PARSER_HPP
#define MAPNIK_IMAGE_FILITER_PARSER_HPP

#include <mapnik/image_filter.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <vector>

namespace mapnik {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

template <typename Iterator, typename ContType>
struct image_filter_grammar :
        qi::grammar<Iterator, ContType(), qi::ascii::space_type>
{
    image_filter_grammar()
        : image_filter_grammar::base_type(start)
    {
        using qi::lit;
        using qi::_val;
        using qi::_1;
        using qi::_a;
        using qi::_b;
        using qi::eps;
        using qi::char_;
        using phoenix::push_back;
        using phoenix::construct;

#if BOOST_VERSION >= 104700
        using qi::no_skip;
        start = -(filter % no_skip[*char_("; ")])
            ;
#else
        start = -(filter)
            ;
#endif

        filter =
            lit("emboss")[push_back(_val,construct<mapnik::filter::emboss>())]
            |
            lit("blur")[push_back(_val,construct<mapnik::filter::blur>())]
            |
            lit("gray")[push_back(_val,construct<mapnik::filter::gray>())]
            |
            lit("edge-detect")[push_back(_val,construct<mapnik::filter::edge_detect>())]
            |
            lit("sobel")[push_back(_val,construct<mapnik::filter::sobel>())]
            |
            lit("sharpen")[push_back(_val,construct<mapnik::filter::sharpen>())]
            |
            lit("x-gradient")[push_back(_val,construct<mapnik::filter::x_gradient>())]
            |
            lit("y-gradient")[push_back(_val,construct<mapnik::filter::y_gradient>())]
            |
            (lit("agg-stack-blur")[_a = 1, _b = 1]
             >> -( lit(':') >> radius_[_a = _1]
                   >> lit(',')
                   >> radius_[_b = _1])
             [push_back(_val,construct<mapnik::filter::agg_stack_blur>(_a,_b))])
            |
            lit("invert")[push_back(_val,construct<mapnik::filter::invert>())]
            ;
    }
    //
    qi::rule<Iterator, ContType(), qi::ascii::space_type> start;
    qi::rule<Iterator, ContType(), qi::locals<int,int>, qi::ascii::space_type> filter;
    qi::uint_parser< unsigned, 10, 1, 3 > radius_;
};

}

#endif // MAPNIK_IMAGE_FILITER_PARSER_HPP
