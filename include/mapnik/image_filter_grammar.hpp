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

#ifndef MAPNIK_IMAGE_FILITER_GRAMMAR_HPP
#define MAPNIK_IMAGE_FILITER_GRAMMAR_HPP

// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#pragma GCC diagnostic pop

// mapnik
#include <mapnik/css_color_grammar.hpp>
#include <mapnik/color.hpp>

// stl
#include <cmath>

namespace mapnik {

namespace filter {
struct color_stop;
struct colorize_alpha;
}

}

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::filter::color_stop,
    (mapnik::color, color )
    (double, offset)
)

namespace mapnik {

namespace qi = boost::spirit::qi;

struct percent_offset_impl
{
    template <typename T>
    struct result
    {
        using type = double;
    };

    double operator() (double val) const
    {
        double result = std::abs(val/100.0);
        if (result > 1.0) result = 1.0;
        return result;
    }
};


template <typename Iterator, typename ContType>
struct image_filter_grammar :
        qi::grammar<Iterator, ContType(), qi::ascii::space_type>
{
    image_filter_grammar();
    qi::rule<Iterator, ContType(), qi::ascii::space_type> start;
    qi::rule<Iterator, ContType(), qi::ascii::space_type> filter;
    qi::rule<Iterator, qi::locals<int,int>, void(ContType&), qi::ascii::space_type> agg_blur_filter;
    qi::rule<Iterator, qi::locals<double,double,double,double,double,double,double,double>,
             void(ContType&), qi::ascii::space_type> scale_hsla_filter;
    qi::rule<Iterator, qi::locals<mapnik::filter::colorize_alpha, mapnik::filter::color_stop>, void(ContType&), qi::ascii::space_type> colorize_alpha_filter;
    qi::rule<Iterator, qi::ascii::space_type> no_args;
    qi::uint_parser< unsigned, 10, 1, 3 > radius_;
    css_color_grammar<Iterator> css_color_;
    qi::rule<Iterator,void(mapnik::filter::color_stop &),qi::ascii::space_type> color_stop_offset;
    phoenix::function<percent_offset_impl> percent_offset;
    qi::rule<Iterator, qi::locals<color>, void(ContType&), qi::ascii::space_type> color_to_alpha_filter;
};

}

#endif // MAPNIK_IMAGE_FILITER_PARSER_HPP
