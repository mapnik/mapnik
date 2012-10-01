/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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
#include <mapnik/util/path_iterator.hpp>
#include <mapnik/util/container_adapter.hpp>

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

namespace mapnik { namespace json {

namespace karma = boost::spirit::karma;
namespace phoenix = boost::phoenix;

namespace {

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

struct multi_geometry_type
{
    template <typename T>
    struct result { typedef boost::tuple<unsigned,bool> type; };

    boost::tuple<unsigned,bool> operator() (geometry_container const& geom) const
    {
        unsigned type = 0u;
        bool collection = false;

        geometry_container::const_iterator itr = geom.begin();
        geometry_container::const_iterator end = geom.end();

        for ( ; itr != end; ++itr)
        {
            if (type != 0u && itr->type() != type)
            {
                collection = true;
                break;
            }
            type = itr->type();
        }
        if (geom.size() > 1) type +=3;
        return boost::tuple<unsigned,bool>(type, collection);
    }
};


template <typename T>
struct json_coordinate_policy : karma::real_policies<T>
{
    typedef boost::spirit::karma::real_policies<T> base_type;
    static int floatfield(T n) { return base_type::fmtflags::fixed; }
    static unsigned precision(T n) { return 12 ;}
};

}

template <typename OutputIterator>
struct geometry_generator_grammar :
        karma::grammar<OutputIterator, geometry_type const& ()>
{

    geometry_generator_grammar()
        : geometry_generator_grammar::base_type(coordinates)
    {
        using boost::spirit::karma::uint_;
        using boost::spirit::karma::_val;
        using boost::spirit::karma::_1;
        using boost::spirit::karma::lit;
        using boost::spirit::karma::_a;
        using boost::spirit::karma::_r1;
        using boost::spirit::karma::eps;
        using boost::spirit::karma::string;

        coordinates =  point | linestring | polygon
            ;

        point = &uint_(mapnik::Point)[_1 = _type(_val)]
            << point_coord [_1 = _first(_val)]
            ;

        linestring = &uint_(mapnik::LineString)[_1 = _type(_val)]
            << lit('[')
            << coords
            << lit(']')
            ;

        polygon = &uint_(mapnik::Polygon)[_1 = _type(_val)]
            << lit('[')
            << coords2
            << lit("]]")
            ;

        point_coord = &uint_
            << lit('[')
            << coord_type << lit(',') << coord_type
            << lit("]")
            ;

        polygon_coord %= ( &uint_(mapnik::SEG_MOVETO) << eps[_r1 += 1]
                           << string[ if_ (_r1 > 1) [_1 = "],["]
                                      .else_[_1 = '[' ]] | &uint_ << lit(','))
            << lit('[') << coord_type
            << lit(',')
            << coord_type << lit(']')
            ;

        coords2 %= *polygon_coord(_a)
            ;

        coords = point_coord % lit(',')
            ;

    }
    // rules
    karma::rule<OutputIterator, geometry_type const& ()> coordinates;
    karma::rule<OutputIterator, geometry_type const& ()> point;
    karma::rule<OutputIterator, geometry_type const& ()> linestring;
    karma::rule<OutputIterator, geometry_type const& ()> polygon;

    karma::rule<OutputIterator, geometry_type const& ()> coords;
    karma::rule<OutputIterator, karma::locals<unsigned>, geometry_type const& ()> coords2;
    karma::rule<OutputIterator, geometry_type::value_type ()> point_coord;
    karma::rule<OutputIterator, geometry_type::value_type (unsigned& )> polygon_coord;

    // phoenix functions
    phoenix::function<get_type > _type;
    phoenix::function<get_first> _first;
    //
    karma::real_generator<double, json_coordinate_policy<double> > coord_type;

};


template <typename OutputIterator>
struct multi_geometry_generator_grammar :
        karma::grammar<OutputIterator, karma::locals<boost::tuple<unsigned,bool> >,
                       geometry_container const& ()>
{

    multi_geometry_generator_grammar()
        : multi_geometry_generator_grammar::base_type(start)
    {
        using boost::spirit::karma::lit;
        using boost::spirit::karma::eps;
        using boost::spirit::karma::_val;
        using boost::spirit::karma::_1;
        using boost::spirit::karma::_a;
        using boost::spirit::karma::_r1;
        using boost::spirit::karma::string;

        geometry_types.add
            (mapnik::Point,"\"Point\"")
            (mapnik::LineString,"\"LineString\"")
            (mapnik::Polygon,"\"Polygon\"")
            (mapnik::Point + 3,"\"MultiPoint\"")
            (mapnik::LineString + 3,"\"MultiLineString\"")
            (mapnik::Polygon + 3,"\"MultiPolygon\"")
            ;

        start %= ( eps(phoenix::at_c<1>(_a))[_a = _multi_type(_val)]
                   << lit("{\"type\":\"GeometryCollection\",\"geometries\":[")
                   << geometry_collection << lit("]}")
                   |
                   geometry)
            ;

        geometry_collection = -(geometry2 % lit(','))
            ;

        geometry = (lit("{\"type\":")
                    << geometry_types[_1 = phoenix::at_c<0>(_a)][_a = _multi_type(_val)]
                    << lit(",\"coordinates\":")
                    << string[ if_ (phoenix::at_c<0>(_a) > 3) [_1 = '[']]
                    << coordinates
                    << string[ if_ (phoenix::at_c<0>(_a) > 3) [_1 = ']']]
                    << lit('}')) | lit("null")
            ;

        geometry2 = lit("{\"type\":")
            << geometry_types[_1 = _a][_a = type_(_val)]
            << lit(",\"coordinates\":")
            << path
            << lit('}')
            ;

        coordinates %= path % lit(',')
            ;

    }
    // rules
    karma::rule<OutputIterator, karma::locals<boost::tuple<unsigned,bool> >,
                geometry_container const&()> start;
    karma::rule<OutputIterator, karma::locals<boost::tuple<unsigned,bool> >,
                geometry_container const&()> geometry_collection;
    karma::rule<OutputIterator, karma::locals<boost::tuple<unsigned,bool> >,
                geometry_container const&()> geometry;
    karma::rule<OutputIterator, karma::locals<unsigned>,
                geometry_type const&()> geometry2;
    karma::rule<OutputIterator, geometry_container const&()> coordinates;
    geometry_generator_grammar<OutputIterator>  path;
    // phoenix
    phoenix::function<multi_geometry_type> _multi_type;
    phoenix::function<get_type > type_;
    // symbols table
    karma::symbols<unsigned, char const*> geometry_types;
};

}}

#endif // MAPNIK_JSON_GEOMETRY_GENERATOR_GRAMMAR_HPP
