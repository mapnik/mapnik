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

#ifndef MAPNIK_GEOMETRY_WKT_GENERATOR_HPP
#define MAPNIK_GEOMETRY_WKT_GENERATOR_HPP

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_type.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/math/special_functions/trunc.hpp> // for vc++ and android whose c++11 libs lack std::trunc
#include <boost/spirit/home/karma/domain.hpp>
#pragma GCC diagnostic pop

namespace mapnik { namespace wkt {

namespace karma = boost::spirit::karma;
namespace phoenix = boost::phoenix;

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
    karma::grammar<OutputIterator, Geometry const& ()>
{
    using coord_type = typename Geometry::value_type;
    wkt_generator_grammar();
    // rules
    karma::rule<OutputIterator, Geometry const&()> geometry;
    karma::rule<OutputIterator, karma::locals<mapnik::geometry::geometry_types>, Geometry const&() > geometry_dispatch;
    karma::rule<OutputIterator, geometry::geometry<coord_type> const&()> point;
    karma::rule<OutputIterator, geometry::point<coord_type> const&()> point_coord;
    karma::rule<OutputIterator, geometry::geometry<coord_type> const&()> linestring;
    karma::rule<OutputIterator, geometry::line_string<coord_type> const&()> linestring_coord;
    karma::rule<OutputIterator, geometry::geometry<coord_type> const&()> polygon;
    karma::rule<OutputIterator, geometry::polygon<coord_type> const&()> polygon_coord;
    karma::rule<OutputIterator, geometry::linear_ring<coord_type> const&()> exterior_ring_coord;
    karma::rule<OutputIterator, std::vector<geometry::linear_ring<coord_type> > const&()> interior_ring_coord;
    karma::rule<OutputIterator, geometry::geometry<coord_type> const& ()> multi_point;
    karma::rule<OutputIterator, geometry::multi_point<coord_type> const& ()> multi_point_coord;
    karma::rule<OutputIterator, geometry::geometry<coord_type> const& ()> multi_linestring;
    karma::rule<OutputIterator, geometry::multi_line_string<coord_type> const& ()> multi_linestring_coord;
    karma::rule<OutputIterator, geometry::geometry<coord_type> const& ()> multi_polygon;
    karma::rule<OutputIterator, geometry::multi_polygon<coord_type> const& ()> multi_polygon_coord;
    karma::rule<OutputIterator, geometry::geometry<coord_type> const& ()> geometry_collection;
    karma::rule<OutputIterator, geometry::geometry_collection<coord_type> const& ()> geometries;
    boost::phoenix::function<detail::get_type<Geometry> > geometry_type;
    karma::symbols<mapnik::geometry::geometry_types, char const*> empty;
    //
    typename detail::coordinate_generator<coord_type>::generator coordinate;
};

}}


#endif // MAPNIK_GEOMETRY_WKT_GENERATOR_HPP
