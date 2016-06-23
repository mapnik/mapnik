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
#pragma GCC diagnostic pop

namespace mapnik { namespace json {

namespace qi = boost::spirit::qi;

template <typename Iterator, typename Boxes, typename ErrorHandler = error_handler<Iterator> >
struct extract_bounding_box_grammar :
        qi::grammar<Iterator, void(Boxes&), space_type>
{
    using position_type = mapnik::geometry::point<double>;
    using boxes_type = Boxes;
    using box_type = typename Boxes::value_type::first_type;
    extract_bounding_box_grammar();
    // rules
    qi::rule<Iterator, void(boxes_type&), space_type> start;
    qi::rule<Iterator, qi::locals<Iterator>, void(boxes_type&), space_type> features;
    qi::rule<Iterator, qi::locals<int, box_type>, void(boxes_type&, Iterator const&), space_type> feature;
    qi::rule<Iterator, qi::locals<box_type>, box_type(), space_type> coords;
    qi::rule<Iterator, boost::optional<position_type>(), space_type> pos;
    qi::rule<Iterator, void(box_type&), space_type> ring;
    qi::rule<Iterator, void(box_type&), space_type> rings;
    qi::rule<Iterator, void(box_type&), space_type> rings_array;
    // generic JSON support
    json::generic_json<Iterator> json;
};

}}

#endif // MAPNIK_JSON_EXTRACT_BOUNDING_BOX_GRAMMAR_HPP
