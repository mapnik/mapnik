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

#ifndef MAPNIK_JSON_EXTRACT_BOUNDING_BOX_GRAMMAR_HPP
#define MAPNIK_JSON_EXTRACT_BOUNDING_BOX_GRAMMAR_HPP

// mapnik
#include <mapnik/json/generic_json.hpp>
#include <mapnik/json/error_handler.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/geometry.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#pragma GCC diagnostic pop

// stl
#include <tuple>

namespace mapnik { namespace json {

using position_type = mapnik::geometry::point<double>;
using boxes_type = std::vector<std::pair<box2d<double>, std::pair<std::size_t, std::size_t>>>;

namespace qi = boost::spirit::qi;

struct calculate_bounding_box_impl
{
    using result_type = void;
    template <typename T0, typename T1>
    result_type operator() (T0 & bbox, T1 const& pos) const
    {
        if (pos)
        {
            double x = pos->x;
            double y = pos->y;
            if (!bbox.valid())
            {
                bbox.init(x, y, x, y); //TODO: add init(x,y) convinience method
            }
            else
            {
                bbox.expand_to_include(x, y);
            }
        }
    }
};

struct push_box_impl
{
    using result_type = void;
    template <typename T0, typename T1, typename T2, typename T3>
    void operator() (T0 & boxes, T1 const& begin, T2 const& box, T3 const& range) const
    {
        if (box.valid()) boxes.emplace_back(box, std::make_pair(std::distance(begin, range.begin()), std::distance(range.begin(), range.end())));
    }
};

template <typename Iterator, typename ErrorHandler = error_handler<Iterator> >
struct extract_bounding_box_grammar :
        qi::grammar<Iterator, void(boxes_type&), space_type>
{
    extract_bounding_box_grammar();
    // rules
    qi::rule<Iterator, void(boxes_type&), space_type> start;
    qi::rule<Iterator, qi::locals<Iterator>, void(boxes_type&), space_type> features;
    qi::rule<Iterator, qi::locals<int, box2d<double>>, void(boxes_type&, Iterator const&), space_type> feature;
    qi::rule<Iterator, qi::locals<box2d<double>>, box2d<double>(), space_type> coords;
    qi::rule<Iterator, boost::optional<position_type>(), space_type> pos;
    qi::rule<Iterator, void(box2d<double>&), space_type> ring;
    qi::rule<Iterator, void(box2d<double>&), space_type> rings;
    qi::rule<Iterator, void(box2d<double>&), space_type> rings_array;
    // generic JSON support
    json::generic_json<Iterator> json;
    // phoenix functions
    boost::phoenix::function<push_box_impl> push_box;
    boost::phoenix::function<calculate_bounding_box_impl> calculate_bounding_box;
    // error handler
    boost::phoenix::function<ErrorHandler> const error_handler;
};

}}

#endif // MAPNIK_JSON_EXTRACT_BOUNDING_BOX_GRAMMAR_HPP
