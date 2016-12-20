/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2016 Artem Pavlenko
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

#ifndef MAPNIK_JSON_GEOMETRY_GRAMMAR_X3_DEF_HPP
#define MAPNIK_JSON_GEOMETRY_GRAMMAR_X3_DEF_HPP

// mapnik
#include <mapnik/json/geometry_grammar_x3.hpp>
#include <mapnik/json/positions_grammar_x3.hpp>
#include <mapnik/json/generic_json_grammar_x3.hpp>
#include <mapnik/json/unicode_string_grammar_x3.hpp>
#include <mapnik/json/create_geometry.hpp>
#include <mapnik/geometry/fusion_adapted.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/util/windows_c2995.hpp>
// boost
#include <boost/fusion/include/std_pair.hpp>

namespace mapnik { namespace json { namespace grammar {

namespace x3 = boost::spirit::x3;
using x3::lit;
using x3::_pass;
using x3::omit;

auto create_geometry = [](auto const& ctx)
{
    auto const& attr = _attr(ctx);
    mapnik::json::create_geometry_impl()(_val(ctx), std::get<0>(attr), std::get<1>(attr));
};

auto assign_element = [] (auto const& ctx)
{
    _val(ctx) = std::move(_attr(ctx));
};

namespace {
template <typename T>
struct assign_geometry_element_visitor : util::noncopyable
{
    assign_geometry_element_visitor(T & t)
        : t_(t) {}

    void operator() (int const& type)
    {
        std::get<0>(t_) = type;
    }
    void operator () (::mapnik::json::positions const& p)
    {
        std::get<1>(t_) = p;
    }

    T & t_;
};

}

auto assign_geometry_element = [] (auto const& ctx)
{
    assign_geometry_element_visitor<decltype(_val(ctx))> v(_val(ctx));
    mapnik::util::apply_visitor(v, _attr(ctx));
};

auto assign_collection = [] (auto const& ctx)
{
    auto & val = _val(ctx);
    std::get<0>(val) = 7; //GeometryCollection
    std::get<1>(val) = std::move(_attr(ctx));
};

auto push_geometry = [] (auto const& ctx)
{
    _val(ctx).emplace_back(std::move(_attr(ctx)));
};

// start rule
geometry_grammar_type const geometry("Geometry");
// rules
x3::rule<class geometry_element_tag, mapnik::util::variant
         <int,
          mapnik::json::positions,
          mapnik::geometry::geometry_collection<double>> const geometry_element("Geometry Element");
x3::rule<class geometry_tuple_tag, std::pair<int, mapnik::json::positions>> const geometry_tuple("Geometry Tuple");
x3::rule<class geometry_collection_tag, mapnik::geometry::geometry_collection<double>> const geometry_collection("Geometry Collection");
// import positions rule
namespace { auto const& pos = positions_grammar(); }
// import generic JSON key:value rule
namespace { auto const& json_key_value = generic_json_key_value(); }

struct geometry_type_ : x3::symbols<int>
{
    geometry_type_()
    {
        add
            ("\"Point\"",1)
            ("\"LineString\"",2)
            ("\"Polygon\"",3)
            ("\"MultiPoint\"",4)
            ("\"MultiLineString\"",5)
            ("\"MultiPolygon\"",6)
            ("\"GeometryCollection\"",7)
            ;
    }
} geometry_type;

auto const geometry_def = (lit('{') > (geometry_tuple)[create_geometry] > lit('}')) | lit("null")
                 ;

auto const geometry_tuple_def = geometry_element[assign_geometry_element] % lit(',')
                 ;

auto const geometry_element_def =
    (lit("\"type\"") > lit(":") > geometry_type[assign_element])
    |
    (lit("\"coordinates\"") > lit(':') > pos[assign_element])
    |
    (lit("\"geometries\"") > lit(':') > lit('[') > geometry_collection[assign_collection] > lit(']'))
    |
    omit[json_key_value]
    ;

auto const geometry_collection_def = geometry[push_geometry] % lit(',')
                     ;

BOOST_SPIRIT_DEFINE(
    geometry,
    geometry_element,
    geometry_tuple,
    geometry_collection
    );

}}}

namespace mapnik { namespace json {
grammar::geometry_grammar_type const& geometry_grammar()
{
    return grammar::geometry;
}
}}


#endif // MAPNIK_JSON_GEOMETRY_GRAMMAR_X3_DEF_HPP
