/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#include <mapnik/util/variant.hpp>
#include <mapnik/geometry_impl.hpp>
#include <mapnik/geometry_type.hpp>
//#include <mapnik/util/path_iterator.hpp>
//#include <mapnik/util/container_adapter.hpp>
//#include <mapnik/vertex.hpp>    // for CommandType::SEG_MOVETO

// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
//#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/math/special_functions/trunc.hpp> // for vc++ and android whose c++11 libs lack std::trunc
#include <boost/spirit/home/karma/domain.hpp>
#pragma GCC diagnostic pop

//stl
//#include <tuple>

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::new_geometry::polygon,
    (mapnik::new_geometry::linear_ring const&, exterior_ring)
    (std::vector<mapnik::new_geometry::linear_ring> const& , interior_rings)
    )

namespace mapnik { namespace json {

namespace karma = boost::spirit::karma;

namespace detail {

template <typename Geometry>
struct get_type
{
    using result_type = mapnik::new_geometry::geometry_types;
    result_type operator() (Geometry const& geom) const
    {
        return mapnik::new_geometry::geometry_type(geom);
    }
};

struct get_x
{
    using result_type = double;
    double operator() (mapnik::new_geometry::point const& pt) const
    {
        return pt.x;
    }
};

struct get_y
{
    using result_type = double;
    double operator() (mapnik::new_geometry::point const& pt) const
    {
        return pt.y;
    }
};

struct debug
{
    using result_type = void;
    template <typename T0, typename T1>
    void operator() (T0 & first, T1 const& second) const
    {
        std::cerr << typeid(first).name() << std::endl;
        std::cerr << typeid(second).name() << std::endl << std::endl;
    }
};


template <typename Geometry>
struct extract_point
{
    using result_type = mapnik::new_geometry::point;
    result_type const& operator() (Geometry const& geom) const
    {
        return geom.template get<mapnik::new_geometry::point>();
    }
};

template <typename Geometry>
struct extract_linestring
{
    using result_type = mapnik::new_geometry::line_string;
    result_type const& operator() (Geometry const& geom) const
    {
        return geom.template get<mapnik::new_geometry::line_string>();
    }
};

template <typename Geometry>
struct extract_polygon
{
    using result_type = mapnik::new_geometry::polygon;
    result_type const& operator() (Geometry const& geom) const
    {
        return geom.template get<mapnik::new_geometry::polygon>();
    }
};

template <typename Geometry>
struct extract_multipoint
{
    using result_type = mapnik::new_geometry::multi_point;
    result_type const& operator() (Geometry const& geom) const
    {
        return geom.template get<mapnik::new_geometry::multi_point>();
    }
};

template <typename Geometry>
struct extract_multilinestring
{
    using result_type = mapnik::new_geometry::multi_line_string;
    result_type const& operator() (Geometry const& geom) const
    {
        return geom.template get<mapnik::new_geometry::multi_line_string>();
    }
};

template <typename Geometry>
struct extract_multipolygon
{
    using result_type = mapnik::new_geometry::multi_polygon;
    result_type const& operator() (Geometry const& geom) const
    {
        return geom.template get<mapnik::new_geometry::multi_polygon>();
    }
};

template <typename Geometry>
struct extract_collection
{
    using result_type = mapnik::new_geometry::geometry_collection;
    result_type const& operator() (Geometry const& geom) const
    {
        return geom.template get<mapnik::new_geometry::geometry_collection>();
    }
};

template <typename GeometryContainer>
struct not_empty
{
    using result_type = bool;
    result_type operator() (GeometryContainer const& cont) const
    {
        for (auto const& geom : cont)
        {
            if (geom.size() > 0) return true;
        }
        return false;
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
        karma::grammar<OutputIterator, Geometry const& ()>
{

    geometry_generator_grammar();
    karma::rule<OutputIterator, Geometry const& ()> start;
    karma::rule<OutputIterator, karma::locals<new_geometry::geometry_types>, Geometry const&()> geometry;
    karma::rule<OutputIterator, void(new_geometry::geometry_types, Geometry const&) > coordinates;
    karma::rule<OutputIterator, new_geometry::point()> point;
    karma::rule<OutputIterator, new_geometry::line_string()> linestring;
    karma::rule<OutputIterator, new_geometry::polygon()> polygon;
    karma::rule<OutputIterator, new_geometry::multi_point ()> multi_point;
    karma::rule<OutputIterator, new_geometry::multi_line_string()> multi_linestring;
    karma::rule<OutputIterator, new_geometry::multi_polygon()> multi_polygon;
    karma::rule<OutputIterator, new_geometry::geometry_collection()> geometry_collection;
    boost::phoenix::function<detail::get_type<Geometry> > geometry_type;
    boost::phoenix::function<detail::get_x> get_x;
    boost::phoenix::function<detail::get_y> get_y;
    boost::phoenix::function<detail::extract_point<Geometry> > extract_point;
    boost::phoenix::function<detail::extract_linestring<Geometry> > extract_linestring;
    boost::phoenix::function<detail::extract_polygon<Geometry> > extract_polygon;
    boost::phoenix::function<detail::extract_multipoint<Geometry> > extract_multipoint;
    boost::phoenix::function<detail::extract_multilinestring<Geometry> > extract_multilinestring;
    boost::phoenix::function<detail::extract_multipolygon<Geometry> > extract_multipolygon;
    boost::phoenix::function<detail::extract_collection<Geometry> > extract_collection;
    boost::phoenix::function<detail::debug> debug;
    karma::real_generator<double, detail::json_coordinate_policy<double> > coordinate;
    karma::symbols<new_geometry::geometry_types, char const*> geometry_types;
};

}}

#endif // MAPNIK_JSON_GEOMETRY_GENERATOR_GRAMMAR_HPP
