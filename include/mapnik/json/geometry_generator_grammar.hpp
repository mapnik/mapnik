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

#ifndef MAPNIK_JSON_GEOMETRY_GENERATOR_GRAMMAR_HPP
#define MAPNIK_JSON_GEOMETRY_GENERATOR_GRAMMAR_HPP

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_type.hpp>
#include <mapnik/geometry_fusion_adapted.hpp>
// boost
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/math/special_functions/trunc.hpp> // for vc++ and android whose c++11 libs lack std::trunc
#include <boost/spirit/home/karma/domain.hpp>
#pragma GCC diagnostic pop

namespace mapnik { namespace json {

namespace karma = boost::spirit::karma;

namespace detail {

template <typename Geometry>
struct get_type
{
    using result_type = mapnik::geometry::geometry_types;
    template <typename T>
    result_type operator() (T const& geom) const
    {
        auto type = mapnik::geometry::geometry_type(geom);
        return type;
    }
};

template <typename T>
struct json_coordinate_policy : karma::real_policies<T>
{
    using base_type = boost::spirit::karma::real_policies<T>;
    static int floatfield(T) { return base_type::fmtflags::fixed; }

    static unsigned precision(T n)
    {
        if (n == 0.0) return 0;
        using namespace boost::spirit;
        return static_cast<unsigned>(14 - boost::math::trunc(log10(traits::get_absolute_value(n))));
    }

    template <typename OutputIterator>
    static bool dot(OutputIterator& sink, T n, unsigned precision)
    {
        if (n == 0) return true;
        return base_type::dot(sink, n, precision);
    }

    template <typename OutputIterator>
    static bool fraction_part(OutputIterator& sink, T n
                              , unsigned adjprec, unsigned precision)
    {
        if (n == 0) return true;
        return base_type::fraction_part(sink, n, adjprec, precision);
    }
};

}

template <typename OutputIterator, typename Geometry>
struct geometry_generator_grammar :
        karma::grammar<OutputIterator, Geometry const&()>
{
    geometry_generator_grammar();
    karma::rule<OutputIterator, Geometry const&()> geometry;
    karma::rule<OutputIterator, karma::locals<mapnik::geometry::geometry_types>, Geometry const&() > geometry_dispatch;
    karma::rule<OutputIterator, geometry::geometry<double> const&()> point;
    karma::rule<OutputIterator, geometry::point<double> const&()> point_coord;
    karma::rule<OutputIterator, geometry::geometry<double> const&()> linestring;
    karma::rule<OutputIterator, geometry::line_string<double> const&()> linestring_coord;
    karma::rule<OutputIterator, geometry::geometry<double> const&()> polygon;
    karma::rule<OutputIterator, geometry::polygon<double> const&()> polygon_coord;
    karma::rule<OutputIterator, geometry::linear_ring<double> const&()> exterior_ring_coord;
    karma::rule<OutputIterator, geometry::polygon<double>::rings_container const&()> interior_ring_coord;
    karma::rule<OutputIterator, geometry::geometry<double> const& ()> multi_point;
    karma::rule<OutputIterator, geometry::multi_point<double> const& ()> multi_point_coord;
    karma::rule<OutputIterator, geometry::geometry<double> const& ()> multi_linestring;
    karma::rule<OutputIterator, geometry::multi_line_string<double> const& ()> multi_linestring_coord;
    karma::rule<OutputIterator, geometry::geometry<double> const& ()> multi_polygon;
    karma::rule<OutputIterator, geometry::multi_polygon<double> const& ()> multi_polygon_coord;
    karma::rule<OutputIterator, geometry::geometry<double> const& ()> geometry_collection;
    karma::rule<OutputIterator, geometry::geometry_collection<double> const& ()> geometries;
    boost::phoenix::function<detail::get_type<Geometry> > geometry_type;
    karma::real_generator<double, detail::json_coordinate_policy<double> > coordinate;

};

}}

#endif // MAPNIK_JSON_GEOMETRY_GENERATOR_GRAMMAR_HPP
