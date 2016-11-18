/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

// mapnik
#include "geojson_index_featureset.hpp"
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/util/utf_conv_win.hpp>
#include <mapnik/util/spatial_index.hpp>
#include <mapnik/geometry/is_empty.hpp>
#include <mapnik/json/unicode_string_grammar_x3_def.hpp>
#include <mapnik/json/positions_grammar_x3_def.hpp>
#include <mapnik/json/generic_json_grammar_x3_def.hpp>
#include <mapnik/json/json_grammar_config.hpp>
#include <mapnik/json/create_geometry.hpp>
// stl
#include <string>
#include <vector>
#include <fstream>


namespace mapnik { namespace json { namespace {

namespace x3 = boost::spirit::x3;
using x3::lit;
using x3::omit;
using x3::char_;

struct feature_tag;
// generic json rule
auto const& value = generic_json_grammar();
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


auto const feature_part = x3::rule<struct feature_part_rule_tag> {} =
    feature_type
    |
    (lit("\"geometry\"") > lit(':') > lit('{') > geometry[assign_geometry] > lit('}'))
    |
    (lit("\"properties\"") > lit(':') > omit[value])
    |
    omit[geojson_string] > lit(':') > omit[value]
    ;

auto const feature = x3::rule<struct feature_rule_tag> {} =
    lit('{') > feature_part % lit(',') > lit('}')
    ;

}}}


geojson_index_featureset::geojson_index_featureset(std::string const& filename, mapnik::filter_in_box const& filter)
    :
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
    //
#elif defined _WINDOWS
    file_(_wfopen(mapnik::utf8_to_utf16(filename).c_str(), L"rb"), std::fclose),
#else
    file_(std::fopen(filename.c_str(),"rb"), std::fclose),
#endif
    ctx_(std::make_shared<mapnik::context_type>())
{

#if defined (MAPNIK_MEMORY_MAPPED_FILE)
    boost::optional<mapnik::mapped_region_ptr> memory =
        mapnik::mapped_memory_cache::instance().find(filename, true);
    if (memory)
    {
        mapped_region_ = *memory;
    }
    else
    {
        throw std::runtime_error("could not create file mapping for " + filename);
    }
#else
    if (!file_) throw std::runtime_error("Can't open " + filename);
#endif
    std::string indexname = filename + ".index";
    std::ifstream index(indexname.c_str(), std::ios::binary);
    if (!index) throw mapnik::datasource_exception("GeoJSON Plugin: can't open index file " + indexname);
    mapnik::util::spatial_index<value_type,
                                mapnik::filter_in_box,
                                std::ifstream>::query(filter, index, positions_);

    std::sort(positions_.begin(), positions_.end(),
              [](value_type const& lhs, value_type const& rhs) { return lhs.first < rhs.first;});
    itr_ = positions_.begin();
}

geojson_index_featureset::~geojson_index_featureset() {}

mapnik::feature_ptr geojson_index_featureset::next()
{
    while( itr_ != positions_.end())
    {
        auto pos = *itr_++;
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
        char const* start = (char const*)mapped_region_->get_address() + pos.first;
        char const*  end = start + pos.second;
#else
        std::fseek(file_.get(), pos.first, SEEK_SET);
        std::vector<char> record;
        record.resize(pos.second);
        std::fread(record.data(), pos.second, 1, file_.get());
        auto const* start = record.data();
        auto const*  end = start + record.size();
#endif
        static const mapnik::transcoder tr("utf8");
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_, feature_id_++));
        using namespace boost::spirit;
        using space_type = mapnik::json::grammar::space_type;
        using mapnik::json::grammar::iterator_type;
        //mapnik::json::geojson_value value;
        //auto const grammar = x3::with<mapnik::json::keys_tag>(std::ref(keys_))
        //    [ mapnik::json::geojson_grammar() ];
        auto grammar = x3::with<mapnik::json::feature_tag>(std::ref(*feature))
            [ mapnik::json::feature ];
        bool result = x3::phrase_parse(start, end, grammar, space_type());

        if (!result) throw std::runtime_error("Failed to parse GeoJSON feature");
        // skip empty geometries
        if (mapnik::geometry::is_empty(feature->get_geometry()))
            continue;
        return feature;
    }
    return mapnik::feature_ptr();
}
