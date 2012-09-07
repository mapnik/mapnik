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

// boost
#include <boost/tuple/tuple.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/home/phoenix/statement/if.hpp>
#include <boost/fusion/include/boost_tuple.hpp>


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

struct get_type
{
    template <typename T>
    struct result { typedef int type; };

    int operator() (geometry_type const& geom) const
    {
        return static_cast<int>(geom.type());
    }
};

struct get_first
{
    template <typename T>
    struct result { typedef geometry_type::value_type const type; };

    geometry_type::value_type const operator() (geometry_type const& geom) const
    {
        geometry_type::value_type coord;
        boost::get<0>(coord) = geom.vertex(0,&boost::get<1>(coord),&boost::get<2>(coord));
        return coord;
    }
};


struct multi_geometry_
{
    template <typename T>
    struct result { typedef bool type; };

    bool operator() (geometry_container const& geom) const
    {
        return geom.size() > 1 ? true : false;
    }
};

struct multi_geometry_type
{
    template <typename T>
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

template <typename OutputIterator>
struct wkt_generator :
    karma::grammar<OutputIterator, geometry_type const& ()>
{
    wkt_generator(bool single = false);
    // rules
    karma::rule<OutputIterator, geometry_type const& ()> wkt;
    karma::rule<OutputIterator, geometry_type const& ()> point;
    karma::rule<OutputIterator, geometry_type const& ()> linestring;
    karma::rule<OutputIterator, geometry_type const& ()> polygon;

    karma::rule<OutputIterator, geometry_type const& ()> coords;
    karma::rule<OutputIterator, karma::locals<unsigned>, geometry_type const& ()> coords2;
    karma::rule<OutputIterator, geometry_type::value_type ()> point_coord;
    karma::rule<OutputIterator, geometry_type::value_type (unsigned& )> polygon_coord;

    // phoenix functions
    phoenix::function<detail::get_type > _type;
    phoenix::function<detail::get_first> _first;
    //
    karma::real_generator<double, detail::wkt_coordinate_policy<double> > coord_type;
};


template <typename OutputIterator>
struct wkt_multi_generator :
        karma::grammar<OutputIterator, karma::locals< boost::tuple<unsigned,bool> >, geometry_container const& ()>
{

    wkt_multi_generator();
    // rules
    karma::rule<OutputIterator, karma::locals<boost::tuple<unsigned,bool> >, geometry_container const& ()> wkt;
    karma::rule<OutputIterator, geometry_container const& ()> geometry;
    karma::rule<OutputIterator, geometry_type const& ()> single_geometry;
    karma::rule<OutputIterator, geometry_container const& ()> multi_geometry;
    wkt_generator<OutputIterator>  path;
    // phoenix
    phoenix::function<detail::multi_geometry_> is_multi;
    phoenix::function<detail::multi_geometry_type> _multi_type;
    phoenix::function<detail::get_type > _type;
    //
    karma::symbols<unsigned, char const*> geometry_types;
};

}}


#endif // MAPNIK_GEOMETRY_WKT_GENERATOR_HPP
