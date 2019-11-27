/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_JSON_GEOJSON_GRAMMAR_X3_DEF_HPP
#define MAPNIK_JSON_GEOJSON_GRAMMAR_X3_DEF_HPP

#include <mapnik/json/geojson_grammar_x3.hpp>
#include <mapnik/json/unicode_string_grammar_x3.hpp>
#include <mapnik/json/positions_grammar_x3.hpp>

#include <boost/fusion/include/std_pair.hpp>

namespace mapnik { namespace json { namespace grammar {

namespace x3 = boost::spirit::x3;

auto make_null = [] (auto const& ctx)
{
    _val(ctx) = mapnik::value_null{};
};

auto make_true = [] (auto const& ctx)
{
    _val(ctx) = true;
};

auto make_false = [] (auto const& ctx)
{
    _val(ctx) = false;
};

auto assign = [](auto const& ctx)
{
    _val(ctx) = std::move(_attr(ctx));
};

auto assign_key = [](auto const& ctx)
{
    std::string const& name = _attr(ctx);
    keys_map & keys = x3::get<keys_tag>(ctx);
    auto result = keys.insert(keys_map::value_type(name, keys.size() + 1));
    std::get<0>(_val(ctx)) = result.first->right;
};

auto assign_value = [](auto const& ctx)
{
    std::get<1>(_val(ctx)) = std::move(_attr(ctx));
};

using x3::lit;
using x3::string;
using x3::lexeme;

struct geometry_type_ : x3::symbols<mapnik::geometry::geometry_types>
{
    geometry_type_()
    {
        add
            ("\"Feature\"",mapnik::geometry::geometry_types(0xff)) // this is a temp hack FIXME
            ("\"Point\"", mapnik::geometry::geometry_types::Point)
            ("\"LineString\"", mapnik::geometry::geometry_types::LineString)
            ("\"Polygon\"", mapnik::geometry::geometry_types::Polygon)
            ("\"MultiPoint\"", mapnik::geometry::geometry_types::MultiPoint)
            ("\"MultiLineString\"", mapnik::geometry::geometry_types::MultiLineString )
            ("\"MultiPolygon\"",mapnik::geometry::geometry_types::MultiPolygon)
            ("\"GeometryCollection\"",mapnik::geometry::geometry_types::GeometryCollection)
            ;
    }
} geometry_type_sym;

// rules
x3::rule<class json_object_tag, geojson_object> const object("JSON Object");
x3::rule<class json_array_tag, geojson_array> const array("JSON Array");
x3::rule<class json_number_tag, json::geojson_value> const number("JSON Number");
x3::rule<class key_value_tag, geojson_object_element> const key_value("JSON key/value");
// GeoJSON
x3::rule<class geojson_coordinates_tag, geojson_object_element> const coordinates("GeoJSON Coordinates");
x3::rule<class geojson_geometry_type_tag, geojson_object_element> const geometry_type("GeoJSON Geometry Type");
auto const geojson_double = x3::real_parser<value_double, x3::strict_real_policies<value_double>>();
auto const geojson_integer = x3::int_parser<value_integer, 10, 1, -1>();

// import unicode string rule
namespace { auto const& geojson_string = unicode_string; }
// import positions rule
namespace { auto const& positions_rule = positions; }

// GeoJSON types
auto const geojson_value_def =  object | array | geojson_string | number
    ;

auto const coordinates_def = lexeme[lit('"') >> (string("coordinates") > lit('"'))][assign_key]
    > lit(':') > (positions_rule[assign_value] | geojson_value[assign_value])
    ;

auto const geometry_type_def = lexeme[lit('"') >> (string("type") > lit('"'))][assign_key]
    > lit(':') > (geometry_type_sym[assign_value] | geojson_value[assign_value])
    ;

auto const key_value_def = geojson_string[assign_key] > lit(':') > geojson_value[assign_value]
    ;

auto const geojson_key_value_def =
    geometry_type
    |
    coordinates
    |
    key_value
    ;

auto const object_def = lit('{')
    > -(geojson_key_value % lit(','))
    > lit('}')
    ;

auto const array_def = lit('[')
    > -(geojson_value % lit(','))
    > lit(']')
    ;

auto const number_def = geojson_double[assign]
    | geojson_integer[assign]
    | lit("true") [make_true]
    | lit ("false") [make_false]
    | lit("null")[make_null]
    ;

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>

BOOST_SPIRIT_DEFINE(
    geojson_value,
    geometry_type,
    coordinates,
    object,
    key_value,
    geojson_key_value,
    array,
    number
    );

#pragma GCC diagnostic pop

}}}

#endif // MAPNIK_JSON_GEOJSON_GRAMMAR_X3_DEF_HPP
