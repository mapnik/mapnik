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

// mapnik
#include <mapnik/image_filter_grammar.hpp>
#include <mapnik/image_filter.hpp>

// boost
#include <boost/version.hpp>

// spirit
#include <boost/spirit/include/phoenix.hpp>

namespace mapnik {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

template <typename Iterator, typename ContType>
image_filter_grammar<Iterator,ContType>::image_filter_grammar()
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
    start = -(filter % no_skip[*char_(", ")])
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
         >> -( lit('(') >> radius_[_a = _1]
               >> lit(',')
               >> radius_[_b = _1]
               >> lit(')'))
         [push_back(_val,construct<mapnik::filter::agg_stack_blur>(_a,_b))])
        |
        lit("invert")[push_back(_val,construct<mapnik::filter::invert>())]
        ;
}

template struct mapnik::image_filter_grammar<std::string::const_iterator,std::vector<mapnik::filter::filter_type> >;

}
