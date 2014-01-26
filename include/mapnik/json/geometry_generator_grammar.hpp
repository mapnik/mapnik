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
#include <mapnik/vertex.hpp>    // for CommandType::SEG_MOVETO

// boost
#include <boost/tuple/tuple.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/math/special_functions/trunc.hpp> // for vc++ and android whose c++11 libs lack std::trunct

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

#ifdef BOOST_SPIRIT_USE_PHOENIX_V3
struct get_type
{
    typedef int result_type;
    result_type operator() (geometry_type const& geom) const
    {
        return static_cast<int>(geom.type());
    }
};

struct get_first
{
    typedef geometry_type::value_type const result_type;
    result_type operator() (geometry_type const& geom) const
    {
        geometry_type::value_type coord;
        std::get<0>(coord) = geom.vertex(0,&std::get<1>(coord),&std::get<2>(coord));
        return coord;
    }
};

struct multi_geometry_type
{
    typedef std::tuple<unsigned,bool>  result_type;
    result_type operator() (geometry_container const& geom) const
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
        return std::tuple<unsigned,bool>(type, collection);
    }
};

struct not_empty
{
    typedef bool result_type;
    result_type operator() (geometry_container const& cont) const
    {
        for (auto const& geom : cont)
        {
            if (geom.size() > 0) return true;
        }
        return false;
    }
};

#else
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
        std::get<0>(coord) = geom.vertex(0,&std::get<1>(coord),&std::get<2>(coord));
        return coord;
    }
};

struct multi_geometry_type
{
    template <typename T>
    struct result { typedef std::tuple<unsigned,bool> type; };

    std::tuple<unsigned,bool> operator() (geometry_container const& geom) const
    {
        unsigned type = 0u;
        bool collection = false;

        geometry_container::const_iterator itr = geom.begin();
        geometry_container::const_iterator end = geom.end();

        for ( ; itr != end; ++itr)
        {
            if (type != 0u && static_cast<unsigned>(itr->type()) != type)
            {
                collection = true;
                break;
            }
            type = itr->type();
        }
        if (geom.size() > 1) type +=3;
        return std::tuple<unsigned,bool>(type, collection);
    }
};


struct not_empty
{
    template <typename T>
    struct result { typedef bool type; };

    bool operator() (geometry_container const& cont) const
    {
        geometry_container::const_iterator itr = cont.begin();
        geometry_container::const_iterator end = cont.end();

        for (; itr!=end; ++itr)
        {
            if (itr->size() > 0) return true;
        }
        return false;
    }
};


#endif


template <typename T>
struct json_coordinate_policy : karma::real_policies<T>
{
    typedef boost::spirit::karma::real_policies<T> base_type;
    static int floatfield(T n) { return base_type::fmtflags::fixed; }

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

template <typename OutputIterator>
struct geometry_generator_grammar :
        karma::grammar<OutputIterator, geometry_type const& ()>
{

    geometry_generator_grammar()
        : geometry_generator_grammar::base_type(coordinates)
    {
        boost::spirit::karma::uint_type uint_;
        boost::spirit::bool_type bool_;
        boost::spirit::karma::_val_type _val;
        boost::spirit::karma::_1_type _1;
        boost::spirit::karma::lit_type lit;
        boost::spirit::karma::_a_type _a;
        boost::spirit::karma::_r1_type _r1;
        boost::spirit::karma::eps_type eps;
        boost::spirit::karma::string_type kstring;

        coordinates =  point | linestring | polygon
            ;

        point = &uint_(mapnik::geometry_type::types::Point)[_1 = _type(_val)]
            << point_coord [_1 = _first(_val)]
            ;

        linestring = &uint_(mapnik::geometry_type::types::LineString)[_1 = _type(_val)]
            << lit('[')
            << coords
            << lit(']')
            ;

        polygon = &uint_(mapnik::geometry_type::types::Polygon)[_1 = _type(_val)]
            << lit('[')
            << coords2
            << lit("]]")
            ;

        point_coord = &uint_
            << lit('[')
            << coord_type << lit(',') << coord_type
            << lit(']')
            ;

        polygon_coord %= ( &uint_(mapnik::SEG_MOVETO) << eps[_r1 += 1]
                           << kstring[ if_ (_r1 > 1) [_1 = "],["]
                                             .else_[_1 = '[' ]]
                           |
                           &uint_(mapnik::SEG_LINETO)
                           << lit(',')) << lit('[') << coord_type << lit(',') << coord_type << lit(']')
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
        karma::grammar<OutputIterator, karma::locals<std::tuple<unsigned,bool> >,
                       geometry_container const& ()>
{

    multi_geometry_generator_grammar()
        : multi_geometry_generator_grammar::base_type(start)
    {
        boost::spirit::karma::uint_type uint_;
        boost::spirit::bool_type bool_;
        boost::spirit::karma::_val_type _val;
        boost::spirit::karma::_1_type _1;
        boost::spirit::karma::lit_type lit;
        boost::spirit::karma::_a_type _a;
        boost::spirit::karma::eps_type eps;
        boost::spirit::karma::string_type kstring;

        geometry_types.add
            (mapnik::geometry_type::types::Point,"\"Point\"")
            (mapnik::geometry_type::types::LineString,"\"LineString\"")
            (mapnik::geometry_type::types::Polygon,"\"Polygon\"")
            (mapnik::geometry_type::types::Point + 3,"\"MultiPoint\"")
            (mapnik::geometry_type::types::LineString + 3,"\"MultiLineString\"")
            (mapnik::geometry_type::types::Polygon + 3,"\"MultiPolygon\"")
            ;

        start %= ( eps(phoenix::at_c<1>(_a))[_a = multi_type_(_val)]
                   << lit("{\"type\":\"GeometryCollection\",\"geometries\":[")
                   << geometry_collection << lit("]}")
                   |
                   geometry)
            ;

        geometry_collection = -(geometry2 % lit(','))
            ;

        geometry = ( &bool_(true)[_1 = not_empty_(_val)] << lit("{\"type\":")
                     << geometry_types[_1 = phoenix::at_c<0>(_a)][_a = multi_type_(_val)]
                     << lit(",\"coordinates\":")
                     << kstring[ phoenix::if_ (phoenix::at_c<0>(_a) > 3) [_1 = '['].else_[_1 = ""]]
                     << coordinates
                     << kstring[ phoenix::if_ (phoenix::at_c<0>(_a) > 3) [_1 = ']'].else_[_1 = ""]]
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
    karma::rule<OutputIterator, karma::locals<std::tuple<unsigned,bool> >,
                geometry_container const&()> start;
    karma::rule<OutputIterator, karma::locals<std::tuple<unsigned,bool> >,
                geometry_container const&()> geometry_collection;
    karma::rule<OutputIterator, karma::locals<std::tuple<unsigned,bool> >,
                geometry_container const&()> geometry;
    karma::rule<OutputIterator, karma::locals<unsigned>,
                geometry_type const&()> geometry2;
    karma::rule<OutputIterator, geometry_container const&()> coordinates;
    geometry_generator_grammar<OutputIterator>  path;
    // phoenix
    phoenix::function<multi_geometry_type> multi_type_;
    phoenix::function<get_type > type_;
    phoenix::function<not_empty> not_empty_;
    // symbols table
    karma::symbols<unsigned, char const*> geometry_types;
};

}}

#endif // MAPNIK_JSON_GEOMETRY_GENERATOR_GRAMMAR_HPP
