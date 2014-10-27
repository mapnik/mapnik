/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

#ifndef MAPNIK_TOPOJSON_GRAMMAR_HPP
#define MAPNIK_TOPOJSON_GRAMMAR_HPP

// mapnik
#include <mapnik/json/error_handler.hpp>
#include <mapnik/json/generic_json.hpp>
#include <mapnik/json/topology.hpp>
#include <mapnik/json/value_converters.hpp>

// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#pragma GCC diagnostic pop

// stl
#include <vector>

namespace mapnik { namespace topojson {

namespace qi = boost::spirit::qi;
namespace fusion = boost::fusion;
namespace standard_wide = boost::spirit::standard_wide;
using standard_wide::space_type;

template <typename Iterator, typename ErrorHandler = json::error_handler<Iterator> >
struct topojson_grammar : qi::grammar<Iterator, space_type, topology()>

{
    topojson_grammar();
private:
    // generic JSON support
    json::generic_json<Iterator> json_;
    // topoJSON
    qi::rule<Iterator, space_type, mapnik::topojson::topology()> topology;
    qi::rule<Iterator, space_type, std::vector<mapnik::topojson::geometry>()> objects;
    qi::rule<Iterator, space_type, std::vector<mapnik::topojson::arc>()> arcs;
    qi::rule<Iterator, space_type, mapnik::topojson::arc()> arc;
    qi::rule<Iterator, space_type, mapnik::topojson::coordinate()> coordinate;
    qi::rule<Iterator, space_type, mapnik::topojson::transform()> transform;
    qi::rule<Iterator, space_type, mapnik::topojson::bounding_box()> bbox;
    qi::rule<Iterator, space_type, mapnik::topojson::geometry() > geometry;
    qi::rule<Iterator, space_type, mapnik::topojson::point()> point;
    qi::rule<Iterator, space_type, mapnik::topojson::multi_point()> multi_point;
    qi::rule<Iterator, space_type, mapnik::topojson::linestring()> linestring;
    qi::rule<Iterator, space_type, mapnik::topojson::multi_linestring()> multi_linestring;
    qi::rule<Iterator, space_type, mapnik::topojson::polygon()> polygon;
    qi::rule<Iterator, space_type, mapnik::topojson::multi_polygon()> multi_polygon;
    qi::rule<Iterator, space_type, void(std::vector<mapnik::topojson::geometry>&)> geometry_collection;

    qi::rule<Iterator, space_type, std::vector<index_type>()> ring;

    // properties
    qi::rule<Iterator, space_type, mapnik::topojson::properties()> properties;
    qi::rule<Iterator, space_type, mapnik::topojson::properties()> attributes;
    qi::rule<Iterator, space_type, mapnik::json::json_value()> attribute_value;
    // id
    qi::rule<Iterator,space_type> id;
    // error handler
    boost::phoenix::function<ErrorHandler> const error_handler;
};

}}

#endif //MAPNIK_TOPOJSON_GRAMMAR_HPP
