/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef MAPNIK_GEOMETRY_WKT_GENERATOR_HPP
#define MAPNIK_GEOMETRY_WKT_GENERATOR_HPP

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/geometry.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/math/special_functions/trunc.hpp> // for vc++ and android whose c++11 libs lack std::trunc
#include <boost/spirit/home/karma/domain.hpp>
MAPNIK_DISABLE_WARNING_POP

namespace mapnik { namespace wkt {

namespace karma = boost::spirit::karma;

namespace detail {

template <typename T>
struct wkt_coordinate_policy : karma::real_policies<T>
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

template <typename T>
struct coordinate_generator;

template <>
struct coordinate_generator<double>
{
    using generator = karma::real_generator<double, detail::wkt_coordinate_policy<double> >;
};

template <>
struct coordinate_generator<std::int64_t>
{
    using generator = karma::int_generator<std::int64_t>;
};

}

template <typename OutputIterator, typename Geometry>
struct wkt_generator_grammar :
    karma::grammar<OutputIterator, Geometry()>
{
    using coordinate_type = typename Geometry::coordinate_type;
    wkt_generator_grammar();
    // rules
    karma::rule<OutputIterator, Geometry()> geometry;
    karma::rule<OutputIterator, geometry::point<coordinate_type>()> point;
    karma::rule<OutputIterator, geometry::point<coordinate_type>()> point_coord;
    karma::rule<OutputIterator, geometry::line_string<coordinate_type>()> linestring;
    karma::rule<OutputIterator, geometry::line_string<coordinate_type>()> linestring_coord;
    karma::rule<OutputIterator, geometry::polygon<coordinate_type>()> polygon;
    karma::rule<OutputIterator, geometry::polygon<coordinate_type>()> polygon_coord;
    karma::rule<OutputIterator, geometry::linear_ring<coordinate_type>()> linear_ring_coord;
    karma::rule<OutputIterator, geometry::multi_point<coordinate_type>()> multi_point;
    karma::rule<OutputIterator, geometry::multi_point<coordinate_type>()> multi_point_coord;
    karma::rule<OutputIterator, geometry::multi_line_string<coordinate_type>()> multi_linestring;
    karma::rule<OutputIterator, geometry::multi_line_string<coordinate_type>()> multi_linestring_coord;
    karma::rule<OutputIterator, geometry::multi_polygon<coordinate_type>()> multi_polygon;
    karma::rule<OutputIterator, geometry::multi_polygon<coordinate_type>()> multi_polygon_coord;
    karma::rule<OutputIterator, geometry::geometry_collection<coordinate_type>()> geometry_collection;
    karma::rule<OutputIterator, geometry::geometry_collection<coordinate_type>()> geometries;
    //
    typename detail::coordinate_generator<coordinate_type>::generator coordinate;
};

}}


#endif // MAPNIK_GEOMETRY_WKT_GENERATOR_HPP
