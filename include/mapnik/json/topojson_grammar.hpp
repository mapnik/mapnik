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

#ifndef MAPNIK_TOPOJSON_GRAMMAR_HPP
#define MAPNIK_TOPOJSON_GRAMMAR_HPP

// mapnik
#include <mapnik/json/error_handler.hpp>
#include <mapnik/json/topology.hpp>
#include <mapnik/json/value_converters.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/qi.hpp>
#pragma GCC diagnostic pop

// stl
#include <vector>

namespace mapnik { namespace topojson {

namespace qi = boost::spirit::qi;
namespace fusion = boost::fusion;
using space_type = mapnik::json::space_type;

template <typename Iterator, typename ErrorHandler = json::error_handler<Iterator> >
struct topojson_grammar : qi::grammar<Iterator, space_type, topology()>

{
    topojson_grammar();
private:
    // generic JSON support
    json::generic_json<Iterator> json;
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
};

}}

#endif //MAPNIK_TOPOJSON_GRAMMAR_HPP
