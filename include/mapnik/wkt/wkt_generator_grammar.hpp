/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
#include <mapnik/vertex.hpp>    // for CommandType::SEG_MOVETO
#include <mapnik/util/container_adapter.hpp>

// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/math/special_functions/trunc.hpp> // for vc++ and android whose c++11 libs lack std::trunc
#pragma GCC diagnostic pop

// stl
#include <tuple>
#include <type_traits>

namespace mapnik { namespace wkt {

namespace karma = boost::spirit::karma;
namespace phoenix = boost::phoenix;

namespace detail {

template <typename Geometry>
struct get_type
{
    using result_type = int;

    int operator() (Geometry const& geom) const
    {
        return static_cast<int>(geom.type());
    }
};

template <typename Geometry>
struct get_first
{
    using result_type = const typename Geometry::value_type;
    typename geometry_type::value_type const operator() (Geometry const& geom) const
    {
        typename Geometry::value_type coord;
        geom.rewind(0);
        std::get<0>(coord) = geom.vertex(&std::get<1>(coord),&std::get<2>(coord));
        return coord;
    }
};

template <typename GeometryContainer>
struct multi_geometry_
{
    using result_type = bool;
    bool operator() (GeometryContainer const& geom) const
    {
        return geom.size() > 1 ? true : false;
    }
};

template <typename T>
struct get_x
{
    using value_type = T;
    using result_type = double;
    double operator() (value_type const& val) const
    {
        return std::get<1>(val);
    }
};

template <typename T>
struct get_y
{
    using value_type = T;
    using result_type = double;
    double operator() (value_type const& val) const
    {
        return std::get<2>(val);
    }
};

template <typename GeometryContainer>
struct multi_geometry_type
{
    using result_type = std::tuple<unsigned,bool>;
    std::tuple<unsigned,bool> operator() (GeometryContainer const& geom) const;
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

}

template <typename OutputIterator, typename Geometry>
struct wkt_generator :
    karma::grammar<OutputIterator, Geometry const& ()>
{
    using geometry_type = Geometry;
    using coord_type = typename std::remove_pointer<typename geometry_type::value_type>::type;

    wkt_generator(bool single = false);
    // rules
    karma::rule<OutputIterator, geometry_type const& ()> wkt;
    karma::rule<OutputIterator, geometry_type const& ()> point;
    karma::rule<OutputIterator, geometry_type const& ()> linestring;
    karma::rule<OutputIterator, geometry_type const& ()> polygon;

    karma::rule<OutputIterator, geometry_type const& ()> coords;
    karma::rule<OutputIterator, karma::locals<unsigned,double,double>, geometry_type const& ()> coords2;
    karma::rule<OutputIterator, coord_type ()> point_coord;
    karma::rule<OutputIterator, karma::locals<double,double>, coord_type (unsigned&, double&, double& )> polygon_coord;

    // phoenix functions
    phoenix::function<detail::get_type<geometry_type> > _type;
    phoenix::function<detail::get_first<geometry_type> > _first;
    phoenix::function<detail::get_x<typename geometry_type::value_type> > _x;
    phoenix::function<detail::get_y<typename geometry_type::value_type> > _y;
    //
    karma::real_generator<double, detail::wkt_coordinate_policy<double> > coordinate;
};


template <typename OutputIterator, typename GeometryContainer>
struct wkt_multi_generator :
        karma::grammar<OutputIterator, karma::locals< std::tuple<unsigned,bool> >, GeometryContainer const& ()>
{
    using geometry_type = typename std::remove_pointer<typename GeometryContainer::value_type>::type;

    wkt_multi_generator();
    // rules
    karma::rule<OutputIterator, karma::locals<std::tuple<unsigned,bool> >, GeometryContainer const& ()> wkt;
    karma::rule<OutputIterator, GeometryContainer const& ()> geometry;
    karma::rule<OutputIterator, geometry_type const& ()> single_geometry;
    karma::rule<OutputIterator, GeometryContainer const& ()> multi_geometry;
    wkt_generator<OutputIterator, geometry_type >  path;
    // phoenix
    phoenix::function<detail::multi_geometry_<GeometryContainer> > is_multi;
    phoenix::function<detail::multi_geometry_type<GeometryContainer> > _multi_type;
    phoenix::function<detail::get_type<geometry_type> > _type;
    //
    karma::symbols<unsigned, char const*> geometry_types;
};

}}


#endif // MAPNIK_GEOMETRY_WKT_GENERATOR_HPP
