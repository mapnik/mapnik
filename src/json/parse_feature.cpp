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

#include <mapnik/json/parse_feature.hpp>
#include <mapnik/json/generic_json_grammar_x3_def.hpp>
#include <mapnik/json/unicode_string_grammar_x3_def.hpp>
#include <mapnik/json/positions_grammar_x3_def.hpp>
#include <mapnik/json/json_grammar_config.hpp>
#include <mapnik/json/create_geometry.hpp>
#include <mapnik/util/conversions.hpp>

#include <mapnik/value.hpp>
#include <mapnik/geometry/geometry_types.hpp>

namespace mapnik { namespace json { namespace {

struct stringifier
{
    std::string operator()(std::string const& val) const
    {
        return "\"" + val + "\"";
    }

    std::string operator()(value_null) const
    {
        return "null";
    }

    std::string operator()(value_bool val) const
    {
        return val ? "true" : "false";
    }

    std::string operator()(value_integer val) const
    {
        std::string str;
        util::to_string(str, val);
        return str;
    }

    std::string operator()(value_double val) const
    {
        std::string str;
        util::to_string(str, val);
        return str;
    }

    std::string operator()(std::vector<mapnik::json::json_value> const& array) const
    {
        std::string str = "[";
        bool first = true;
        for (auto const& val : array)
        {
            if (first) first = false;
            else str += ",";
            str += mapnik::util::apply_visitor(*this, val);
        }
        str += "]";
        return str;
    }

    std::string operator()(std::vector<std::pair<std::string, mapnik::json::json_value>> const& object) const
    {
        std::string str = "{";
        bool first = true;
        for (auto const& kv : object)
        {
            if (first) first = false;
            else str += ",";
            str +=  "\"" + kv.first + "\"";
            str += ":";
            str += mapnik::util::apply_visitor(*this, kv.second);
        }
        str += "}";
        return str;
    }
};

struct attribute_value_visitor
{
public:
    attribute_value_visitor(mapnik::transcoder const& tr)
        : tr_(tr) {}

    mapnik::value operator()(std::string const& val) const
    {
        return mapnik::value(tr_.transcode(val.c_str()));
    }

    mapnik::value operator()(std::vector<mapnik::json::json_value> const& array) const
    {
        std::string str = stringifier()(array);
        return mapnik::value(tr_.transcode(str.c_str()));
    }

    mapnik::value operator()(std::vector<std::pair<std::string, mapnik::json::json_value> > const& object) const
    {
        std::string str = stringifier()(object);
        return mapnik::value(tr_.transcode(str.c_str()));
    }

    template <typename T>
    mapnik::value operator()(T const& val) const
    {
        return mapnik::value(val);
    }

    mapnik::transcoder const& tr_;
};


namespace x3 = boost::spirit::x3;
using x3::lit;
using x3::omit;
using x3::char_;

struct transcoder_tag;
struct feature_tag;
//
auto const& value = mapnik::json::generic_json_grammar();
// import unicode string rule
auto const& geojson_string = unicode_string_grammar();
// import positions rule
auto const& positions_rule = positions_grammar();
// geometry types symbols
struct geometry_type_ : x3::symbols<mapnik::geometry::geometry_types>
{
    geometry_type_()
    {
        add
            ("\"Point\"", mapnik::geometry::geometry_types::Point)
            ("\"LineString\"", mapnik::geometry::geometry_types::LineString)
            ("\"Polygon\"", mapnik::geometry::geometry_types::Polygon)
            ("\"MultiPoint\"", mapnik::geometry::geometry_types::MultiPoint)
            ("\"MultiLineString\"", mapnik::geometry::geometry_types::MultiLineString )
            ("\"MultiPolygon\"",mapnik::geometry::geometry_types::MultiPolygon)
            ("\"GeometryCollection\"",mapnik::geometry::geometry_types::GeometryCollection)
            ;
    }
} geometry_type_symbols;

auto assign_name = [](auto const& ctx)
{
    std::get<0>(_val(ctx)) = std::move(_attr(ctx));
};
auto assign_value = [](auto const& ctx)
{
    std::get<1>(_val(ctx)) = std::move(_attr(ctx));
};

auto const assign_geometry_type = [] (auto const& ctx)
{
    std::get<0>(_val(ctx)) = _attr(ctx);
};

auto const assign_geometry = [] (auto const& ctx)
{
    mapnik::feature_impl & feature = x3::get<feature_tag>(ctx);
    mapnik::geometry::geometry<double> geom;
    auto const type = std::get<0>(_attr(ctx));
    auto const& coordinates = std::get<1>(_attr(ctx));
    mapnik::json::create_geometry(geom, type, coordinates);
    feature.set_geometry(std::move(geom));
};

auto const assign_positions = [] (auto const& ctx)
{
    std::get<1>(_val(ctx)) = std::move(_attr(ctx));
};

auto assign_property = [](auto const& ctx)
{
    mapnik::feature_impl & feature = x3::get<feature_tag>(ctx);
    mapnik::transcoder const& tr = x3::get<transcoder_tag>(ctx);
    feature.put_new(std::get<0>(_attr(ctx)),
                    mapnik::util::apply_visitor(attribute_value_visitor(tr),
                                                std::get<1>(_attr(ctx))));
};


auto const feature_type = lit("\"type\"") > lit(':') > lit("\"Feature\"");
auto const geometry_type = x3::rule<struct geometry_type_tag, mapnik::geometry::geometry_types> {} =
    lit("\"type\"") > lit(':') > geometry_type_symbols;

auto const coordinates = x3::rule<struct coordinates_tag, positions> {} =
    lit("\"coordinates\"") > lit(':') > positions_rule
    ;

auto const geometry = x3::rule<struct geomerty_tag, std::tuple<mapnik::geometry::geometry_types, positions>> {} =
    (geometry_type[assign_geometry_type]
     |
     coordinates[assign_positions]
     |
     (omit[geojson_string] > lit(':') > omit[value])) % lit(',')
    ;

auto const property = x3::rule<struct property, std::tuple<std::string, json_value>> {} =
    geojson_string[assign_name] > lit(':') > value[assign_value]
    ;

auto const properties = x3::rule<struct properties_tag> {} =
    property[assign_property] % lit(',')
    ;
auto const feature_part = x3::rule<struct feature_part_rule_tag> {} =
    feature_type
    |
    (lit("\"geometry\"") > lit(':') > lit('{') > geometry[assign_geometry] > lit('}'))
    |
    (lit("\"properties\"") > lit(':') > lit('{') > properties > lit('}'))
    |
    omit[geojson_string] > lit(':') > omit[value]
    ;

auto const feature = x3::rule<struct feature_rule_tag> {} =
    lit('{') > feature_part % lit(',') > lit('}')
    ;

auto const geometry_rule = x3::rule<struct geometry_rule_tag> {} = lit('{') > geometry[assign_geometry] > lit('}');

}

template <typename Iterator>
void parse_feature(Iterator start, Iterator end, feature_impl& feature, mapnik::transcoder const& tr)
{
    using space_type = mapnik::json::grammar::space_type;
    auto grammar = x3::with<mapnik::json::transcoder_tag>(std::ref(tr))
        [x3::with<mapnik::json::feature_tag>(std::ref(feature))
         [ mapnik::json::feature ]];
    if (!x3::phrase_parse(start, end, grammar, space_type()))
    {
        throw std::runtime_error("Can't parser GeoJSON Feature");
    }
}

template <typename Iterator>
void parse_geometry(Iterator start, Iterator end, feature_impl& feature)
{
    using space_type = mapnik::json::grammar::space_type;
    auto grammar = x3::with<mapnik::json::feature_tag>(std::ref(feature))
        [ mapnik::json::geometry_rule ];
    if (!x3::phrase_parse(start, end, grammar, space_type()))
    {
        throw std::runtime_error("Can't parser GeoJSON Geometry");
    }
}

using iterator_type = char const* ;//mapnik::json::grammar::iterator_type;
template void parse_feature<iterator_type>(iterator_type,iterator_type, feature_impl& feature, mapnik::transcoder const& tr);
template void parse_geometry<iterator_type>(iterator_type,iterator_type, feature_impl& feature);

}}
