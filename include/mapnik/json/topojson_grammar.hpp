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

struct create_point
{
    using result_type = mapnik::topojson::point;
    template <typename T0, typename T1>
    result_type operator()(T0 & coord, T1 & props) const
    {
        mapnik::topojson::point pt;
        if (coord.template is<mapnik::topojson::coordinate>())
        {
            auto const& coord_ = coord.template get<mapnik::topojson::coordinate>();
            pt.coord = coord_;
            pt.props = props;
        }
        return pt;
    }
};

struct create_multi_point
{
    using result_type = mapnik::topojson::multi_point;
    template <typename T0, typename T1>
    result_type operator()(T0 & coords, T1 & props) const
    {
        mapnik::topojson::multi_point mpt;
        if (coords.template is<std::vector<mapnik::topojson::coordinate>>())
        {
            auto const& points = coords.template get<std::vector<mapnik::topojson::coordinate>>();
            mpt. points = points;
            mpt.props = props;
        }
        return mpt;
    }
};

struct create_line_string
{
    using result_type = mapnik::topojson::linestring;
    template <typename T0, typename T1>
    result_type operator()(T0 & arcs, T1 & props) const
    {
        mapnik::topojson::linestring line;
        if (arcs.template is<std::vector<index_type>>())
        {
            auto const& arcs_ = arcs.template get<std::vector<index_type>>();
            line.rings = arcs_;
            line.props = props;
        }
        return line;
    }
};

struct create_multi_line_string
{
    using result_type = mapnik::topojson::multi_linestring;
    template <typename T0, typename T1>
    result_type operator()(T0 & arcs, T1 & props) const
    {
        mapnik::topojson::multi_linestring mline;
        if (arcs.template is<std::vector<std::vector<index_type>>>())
        {
            auto const& arcs_ = arcs.template get<std::vector<std::vector<index_type>>>();
            mline.lines = arcs_;
            mline.props = props;
        }
        return mline;
    }
};

struct create_polygon
{
    using result_type = mapnik::topojson::polygon;
    template <typename T0, typename T1>
    result_type operator()(T0 & arcs, T1 & props) const
    {
        mapnik::topojson::polygon poly;
        if (arcs.template is<std::vector<std::vector<index_type>>>())
        {
            auto const& arcs_ = arcs.template get<std::vector<std::vector<index_type>>>();
            poly.rings = arcs_;
            poly.props = props;
        }
        return poly;
    }
};

struct create_multi_polygon
{
    using result_type = mapnik::topojson::multi_polygon;
    template <typename T0, typename T1>
    result_type operator()(T0 & arcs, T1 & props) const
    {
        mapnik::topojson::multi_polygon mpoly;
        if (arcs.template is<std::vector<std::vector<std::vector<index_type>>>>())
        {
            auto const& arcs_ = arcs.template get<std::vector<std::vector<std::vector<index_type>>>>();
            mpoly.polygons = arcs_;
            mpoly.props = props;
        }
        return mpoly;
    }
};


struct create_geometry_impl
{
    using result_type = mapnik::topojson::geometry;
    template <typename T0, typename T1, typename T2, typename T3>
    result_type operator()(T0 geom_type, T1 & coord, T2 & arcs, T3 & props) const
    {
        switch (geom_type)
        {
        case 1: //Point
            return create_point()(coord, props);
        case 2: //LineString
            return create_line_string()(arcs, props);
        case 3: //Polygon
            return create_polygon()(arcs, props);
        case 4: //MultiPoint
            return create_multi_point()(coord, props);
        case 5: //MultiLineString
            return create_multi_line_string()(arcs, props);
        case 6: //MultiPolygon
            return create_multi_polygon()(arcs, props);
        default:
            break;
        }
        return mapnik::topojson::geometry(); //empty
    }
};

using coordinates_type = util::variant<coordinate,std::vector<coordinate>>;
using arcs_type = util::variant<std::vector<index_type>, std::vector<std::vector<index_type>>, std::vector<std::vector<std::vector<index_type>>>>;
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
    qi::rule<Iterator, space_type, mapnik::topojson::coordinate()> coordinate_;
    qi::rule<Iterator, space_type, coordinates_type()> coordinates;
    qi::rule<Iterator, space_type, mapnik::topojson::transform()> transform;
    qi::rule<Iterator, space_type, mapnik::topojson::bounding_box()> bbox;
    qi::rule<Iterator, qi::locals<int, coordinates_type, arcs_type, properties>, mapnik::topojson::geometry(), space_type> geometry;
    qi::rule<Iterator, space_type, void(std::vector<mapnik::topojson::geometry>&)> geometry_collection;
    qi::rule<Iterator, space_type, std::vector<index_type>()> ring;
    qi::rule<Iterator, space_type, std::vector<std::vector<index_type>>()> rings;
    qi::rule<Iterator, space_type, arcs_type()> rings_array;
    // properties
    qi::rule<Iterator, space_type, mapnik::topojson::properties()> properties_;
    qi::symbols<char, int> geometry_type_dispatch;
};

}}

#endif //MAPNIK_TOPOJSON_GRAMMAR_HPP
