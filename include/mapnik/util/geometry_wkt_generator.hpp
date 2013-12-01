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

// boost
#include <boost/tuple/tuple.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/fusion/include/boost_tuple.hpp>
#include <boost/type_traits/remove_pointer.hpp>

#include <boost/math/special_functions/trunc.hpp> // trunc to avoid needing C++11

namespace boost { namespace spirit { namespace traits {

// make gcc and darwin toolsets happy.
template <>
struct is_container<mapnik::geometry_container>
    : mpl::false_
{};

}}}

namespace mapnik { namespace util {

namespace karma = boost::spirit::karma;
namespace phoenix = boost::phoenix;

namespace detail {

template <typename Geometry>
struct get_type
{
    template <typename T>
    struct result { typedef int type; };

    int operator() (Geometry const& geom) const
    {
        return static_cast<int>(geom.type());
    }
};

template <typename T>
struct get_first
{
    typedef T geometry_type;

    template <typename U>
    struct result { typedef typename geometry_type::value_type const type; };

    typename geometry_type::value_type const operator() (geometry_type const& geom) const
    {
        typename geometry_type::value_type coord;
        geom.rewind(0);
        boost::get<0>(coord) = geom.vertex(&boost::get<1>(coord),&boost::get<2>(coord));
        return coord;
    }
};

template <typename T>
struct multi_geometry_
{
    typedef T geometry_container;

    template <typename U>
    struct result { typedef bool type; };
    bool operator() (geometry_container const& geom) const
    {
        return geom.size() > 1 ? true : false;
    }
};

template <typename T>
struct get_x
{
    typedef T value_type;

    template <typename U>
    struct result { typedef double type; };

    double operator() (value_type const& val) const
    {
        return boost::get<1>(val);
    }
};

template <typename T>
struct get_y
{
    typedef T value_type;

    template <typename U>
    struct result { typedef double type; };

    double operator() (value_type const& val) const
    {
        return boost::get<2>(val);
    }
};

template <typename T>
struct multi_geometry_type
{
    typedef T geometry_container;

    template <typename U>
    struct result { typedef boost::tuple<unsigned,bool> type; };

    boost::tuple<unsigned,bool> operator() (geometry_container const& geom) const;
};


template <typename T>
struct wkt_coordinate_policy : karma::real_policies<T>
{
    typedef boost::spirit::karma::real_policies<T> base_type;
    static int floatfield(T n) { return base_type::fmtflags::fixed; }
    static unsigned precision(T n)
    {
        if (n == 0.0) return 0;
        return 6;
        //using namespace boost::spirit; // for traits
        //return std::max(6u, static_cast<unsigned>(15 - boost::math::trunc(log10(traits::get_absolute_value(n)))));
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
    typedef Geometry geometry_type;
    typedef typename boost::remove_pointer<typename geometry_type::value_type>::type coord_type;

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
        karma::grammar<OutputIterator, karma::locals< boost::tuple<unsigned,bool> >, GeometryContainer const& ()>
{
    typedef typename boost::remove_pointer<typename GeometryContainer::value_type>::type geometry_type;

    wkt_multi_generator();
    // rules
    karma::rule<OutputIterator, karma::locals<boost::tuple<unsigned,bool> >, GeometryContainer const& ()> wkt;
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
