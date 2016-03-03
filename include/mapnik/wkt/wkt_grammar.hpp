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

#ifndef MAPNIK_WKT_GRAMMAR_HPP
#define MAPNIK_WKT_GRAMMAR_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/geometry.hpp>
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/assert.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#pragma GCC diagnostic pop


namespace mapnik { namespace wkt {

using namespace boost::spirit;

namespace detail {

struct assign
{
    using result_type = void;
    template <typename T0, typename T1>
    void operator() (T0 & geom, T1 && obj) const
    {
        geom = std::move(obj);
    }
};

struct move_part
{
    using result_type = void;
    template <typename Geometry, typename Part>
    void operator() (Geometry & geom, Part && part) const
    {
        geom.push_back(std::move(part));
    }
};

struct set_exterior
{
    using result_type = void;
    template <typename Polygon, typename Ring>
    void operator() (Polygon & poly, Ring && ring) const
    {
        poly.set_exterior_ring(std::move(ring));
    }
};

struct add_hole
{
    using result_type = void;
    template <typename Polygon, typename Ring>
    void operator() (Polygon & poly, Ring && ring) const
    {
        poly.add_hole(std::move(ring));
    }
};

}

template <typename Iterator>
struct wkt_grammar : qi::grammar<Iterator, void(mapnik::geometry::geometry<double> &) , ascii::space_type>
{
    wkt_grammar();
    qi::rule<Iterator, void(mapnik::geometry::geometry<double> &), ascii::space_type> geometry_tagged_text;
    qi::rule<Iterator, void(mapnik::geometry::geometry<double> &), ascii::space_type> point_tagged_text;
    qi::rule<Iterator, void(mapnik::geometry::geometry<double> &), ascii::space_type> linestring_tagged_text;
    qi::rule<Iterator, void(mapnik::geometry::geometry<double> &), ascii::space_type> polygon_tagged_text;
    qi::rule<Iterator, void(mapnik::geometry::geometry<double> &), ascii::space_type> multipoint_tagged_text;
    qi::rule<Iterator, void(mapnik::geometry::geometry<double> &), ascii::space_type> multilinestring_tagged_text;
    qi::rule<Iterator, void(mapnik::geometry::geometry<double> &), ascii::space_type> multipolygon_tagged_text;
    qi::rule<Iterator, void(mapnik::geometry::geometry<double> &), ascii::space_type> geometrycollection_tagged_text;
    qi::rule<Iterator, mapnik::geometry::point<double>(), ascii::space_type> point_text;
    qi::rule<Iterator, mapnik::geometry::line_string<double>(), ascii::space_type> linestring_text;
    qi::rule<Iterator, mapnik::geometry::linear_ring<double>(), ascii::space_type> linearring_text;
    qi::rule<Iterator, mapnik::geometry::polygon<double>(), ascii::space_type> polygon_text;
    qi::rule<Iterator, mapnik::geometry::multi_point<double>(), ascii::space_type> multipoint_text;
    qi::rule<Iterator, mapnik::geometry::multi_line_string<double>(), ascii::space_type> multilinestring_text;
    qi::rule<Iterator, mapnik::geometry::multi_polygon<double>(), ascii::space_type> multipolygon_text;
    qi::rule<Iterator, qi::locals<mapnik::geometry::geometry<double> >,
             mapnik::geometry::geometry_collection<double>(), ascii::space_type> geometrycollection_text;
    qi::rule<Iterator, mapnik::geometry::point<double>(), ascii::space_type> point;
    qi::rule<Iterator, mapnik::geometry::line_string<double>(), ascii::space_type> points;
    qi::rule<Iterator, mapnik::geometry::linear_ring<double>(), ascii::space_type> ring_points;
    qi::rule<Iterator,ascii::space_type> empty_set;
    boost::phoenix::function<detail::assign> assign;
    boost::phoenix::function<detail::move_part> move_part;
    boost::phoenix::function<detail::set_exterior> set_exterior;
    boost::phoenix::function<detail::add_hole> add_hole;
};

}}

#endif // MAPNIK_WKT_GRAMMAR_HPP
