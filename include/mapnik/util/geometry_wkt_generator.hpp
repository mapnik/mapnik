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

// boost
#include <boost/tuple/tuple.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/home/phoenix/statement/if.hpp>
#include <boost/fusion/include/boost_tuple.hpp>
#include <boost/type_traits/remove_pointer.hpp>

//#define BOOST_SPIRIT_USE_PHOENIX_V3 1

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
    static unsigned precision(T n) { return 6 ;}
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
    karma::rule<OutputIterator, karma::locals<unsigned>, geometry_type const& ()> coords2;
    karma::rule<OutputIterator, coord_type ()> point_coord;
    karma::rule<OutputIterator, coord_type (unsigned& )> polygon_coord;

    // phoenix functions
    phoenix::function<detail::get_type<geometry_type> > _type;
    phoenix::function<detail::get_first<geometry_type> > _first;
    //
    karma::real_generator<double, detail::wkt_coordinate_policy<double> > coordinate;
};


template <typename OutputIterator, typename GeometryContainer>
struct wkt_multi_generator :
        karma::grammar<OutputIterator, karma::locals< boost::tuple<unsigned,bool> >, GeometryContainer const& ()>
{
    typedef GeometryContainer geometry_contaner;
    typedef boost::remove_pointer<typename geometry_container::value_type>::type geometry_type;

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
