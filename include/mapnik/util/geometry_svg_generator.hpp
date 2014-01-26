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

#ifndef MAPNIK_GEOMETRY_SVG_GENERATOR_HPP
#define MAPNIK_GEOMETRY_SVG_GENERATOR_HPP


// mapnik
#include <mapnik/global.hpp>
#include <mapnik/geometry.hpp> // for container stuff
#include <mapnik/ctrans.hpp> // for container stuff
#include <mapnik/util/path_iterator.hpp>
#include <mapnik/util/container_adapter.hpp>

// boost
#include <boost/tuple/tuple.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/type_traits/remove_pointer.hpp>


// adapted to conform to the concepts
// required by Karma to be recognized as a container of
// attributes for output generation.

namespace boost { namespace spirit { namespace traits {

// TODO - this needs to be made generic to any path type
typedef mapnik::coord_transform<mapnik::CoordTransform, mapnik::geometry_type> path_type;

template <>
struct is_container<path_type const> : mpl::true_ {} ;

template <>
struct container_iterator<path_type const>
{
    typedef mapnik::util::path_iterator<path_type> type;
};

template <>
struct begin_container<path_type const>
{
    static mapnik::util::path_iterator<path_type>
    call (path_type const& g)
    {
        return mapnik::util::path_iterator<path_type>(g);
    }
};

template <>
struct end_container<path_type const>
{
    static mapnik::util::path_iterator<path_type>
    call (path_type const& /*g*/)
    {
        return mapnik::util::path_iterator<path_type>();
    }
};

}}}


namespace mapnik { namespace util {

    namespace karma = boost::spirit::karma;
    namespace phoenix = boost::phoenix;

    namespace svg_detail {

#ifdef BOOST_SPIRIT_USE_PHOENIX_V3
    template <typename Geometry>
    struct get_type
    {
        typedef int result_type;
        result_type operator() (Geometry const& geom) const
        {
            return static_cast<int>(geom.type());
        }
    };

    template <typename T>
    struct get_first
    {
        typedef T geometry_type;
        typedef typename geometry_type::value_type const result_type;
        result_type operator() (geometry_type const& geom) const
        {
            typename geometry_type::value_type coord;
            geom.rewind(0);
            std::get<0>(coord) = geom.vertex(&std::get<1>(coord),&std::get<2>(coord));
            return coord;
        }
    };
#else
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

        typename geometry_type::value_type operator() (geometry_type const& geom) const
        {
            typename geometry_type::value_type coord;
            geom.rewind(0);
            std::get<0>(coord) = geom.vertex(&std::get<1>(coord),&std::get<2>(coord));
            return coord;
        }
    };

#endif
    template <typename T>
    struct coordinate_policy : karma::real_policies<T>
    {
        typedef boost::spirit::karma::real_policies<T> base_type;
        static int floatfield(T n) { return base_type::fmtflags::fixed; }
        static unsigned precision(T n) { return 4u ;}
    };
    }

    template <typename OutputIterator, typename Geometry>
    struct svg_generator :
        karma::grammar<OutputIterator, Geometry const& ()>
    {

        typedef Geometry geometry_type;
        typedef typename boost::remove_pointer<typename geometry_type::value_type>::type coord_type;

        svg_generator()
            : svg_generator::base_type(svg)
        {
            boost::spirit::karma::uint_type uint_;
            boost::spirit::karma::_val_type _val;
            boost::spirit::karma::_1_type _1;
            boost::spirit::karma::lit_type lit;
            boost::spirit::karma::_a_type _a;
            boost::spirit::karma::string_type kstring;

            svg = point | linestring | polygon
                ;

            point = &uint_(mapnik::geometry_type::types::Point)[_1 = _type(_val)]
                << svg_point [_1 = _first(_val)]
                ;

            svg_point = &uint_
                << lit("cx=\"") << coordinate
                << lit("\" cy=\"") << coordinate
                << lit('\"')
                ;

            linestring = &uint_(mapnik::geometry_type::types::LineString)[_1 = _type(_val)]
                << lit("d=\"") << svg_path << lit("\"")
                ;

            polygon = &uint_(mapnik::geometry_type::types::Polygon)[_1 = _type(_val)]
                << lit("d=\"") << svg_path << lit("\"")
                ;

            svg_path %= ((&uint_(mapnik::SEG_MOVETO) << lit('M')
                          | &uint_(mapnik::SEG_LINETO) [_a +=1] << kstring [if_(_a == 1) [_1 = "L" ].else_[_1 =""]])
                         << lit(' ') << coordinate << lit(' ') << coordinate) % lit(' ')
                ;



        }
        // rules
        karma::rule<OutputIterator, geometry_type const& ()> svg;
        karma::rule<OutputIterator, geometry_type const& ()> point;
        karma::rule<OutputIterator, geometry_type const& ()> linestring;
        karma::rule<OutputIterator, geometry_type const& ()> polygon;

        karma::rule<OutputIterator, coord_type ()> svg_point;
        karma::rule<OutputIterator, karma::locals<unsigned>, geometry_type const& ()> svg_path;

        // phoenix functions
        phoenix::function<svg_detail::get_type<geometry_type> > _type;
        phoenix::function<svg_detail::get_first<geometry_type> > _first;
        //
        karma::real_generator<double, svg_detail::coordinate_policy<double> > coordinate;

    };

}}

#endif // MAPNIK_GEOMETRY_SVG_GENERATOR_HPP
