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

#include "process_geojson_file_x3.hpp"

#if defined(MAPNIK_MEMORY_MAPPED_FILE)
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <boost/spirit/home/x3.hpp>
#pragma GCC diagnostic pop
#include <mapnik/mapped_memory_cache.hpp>
#else
#include <mapnik/util/file_io.hpp>
#endif
#include <mapnik/feature.hpp>
#include <mapnik/json/json_grammar_config.hpp>
#include <mapnik/json/geojson_grammar_x3_def.hpp>
#include <mapnik/json/unicode_string_grammar_x3_def.hpp>
#include <mapnik/json/positions_grammar_x3_def.hpp>

namespace {

constexpr mapnik::json::well_known_names feature_properties[] = {
    mapnik::json::well_known_names::type,
    mapnik::json::well_known_names::geometry,
    mapnik::json::well_known_names::properties}; // sorted

constexpr mapnik::json::well_known_names geometry_properties[] = {
    mapnik::json::well_known_names::type,
    mapnik::json::well_known_names::coordinates}; // sorted

template <typename Keys>
std::string join(Keys const& keys)
{
    std::string result;
    bool first = true;
    for (auto const& key : keys)
    {
        if (!first) result += ",";
        result += "\"" + std::string(mapnik::json::wkn_to_string(key)) + "\"";
        first = false;
    }
    return result;
}

template <typename Iterator, typename Keys>
bool has_keys(Iterator first1, Iterator last1, Keys const& keys)
{
    auto first2 = std::begin(keys);
    auto last2 = std::end(keys);
    for (; first2 != last2; ++first1)
    {
        if (first1 == last1 || *first2 < std::get<0>(*first1))
            return false;
        if (!(std::get<0>(*first1) < *first2))
            ++first2;
    }
    return true;
}

template <typename Keys>
bool validate_geojson_feature(mapnik::json::geojson_value & value, Keys const& keys, bool verbose)
{
    if (!value.is<mapnik::json::geojson_object>())
    {
        if (verbose) std::clog << "Expecting an GeoJSON object" << std::endl;
        return false;
    }
    mapnik::json::geojson_object & feature = mapnik::util::get<mapnik::json::geojson_object>(value);
    std::sort(feature.begin(), feature.end(), [](auto const& e0, auto const& e1)
              {
                  return std::get<0>(e0) < std::get<0>(e1);
              });

    if (!has_keys(feature.begin(), feature.end(), feature_properties))
    {
        if (verbose) std::clog << "Expecting one of " << join(feature_properties) << std::endl;
        return false;
    }

    for (auto & elem : feature)
    {
        auto const key = std::get<0>(elem);
        if (key == mapnik::json::well_known_names::geometry)
        {
            auto & geom_value = std::get<1>(elem);
            if (!geom_value.is<mapnik::json::geojson_object>())
            {
                if (verbose) std::clog << "\"geometry\": xxx <-- expecting an JSON object here" << std::endl;
                return false;
            }
            auto & geometry = mapnik::util::get<mapnik::json::geojson_object>(geom_value);
            // sort by property name
            std::sort(geometry.begin(), geometry.end(), [](auto const& e0, auto const& e1)
                      {
                          return std::get<0>(e0) < std::get<0>(e1);
                      });
            if (!has_keys(geometry.begin(), geometry.end(), geometry_properties))
            {
                if (verbose) std::clog << "Expecting one of " << join(geometry_properties) << std::endl;
                return false;
            }

            mapnik::geometry::geometry_types geom_type;
            mapnik::json::positions const* coordinates = nullptr;
            for (auto & elem2 : geometry)
            {
                auto const key2 = std::get<0>(elem2);
                if (key2 == mapnik::json::well_known_names::type)
                {
                    auto const& geom_type_value = std::get<1>(elem2);
                    if (!geom_type_value.is<mapnik::geometry::geometry_types>())
                    {
                        if (verbose) std::clog << "\"type\": xxx <-- expecting an GeoJSON geometry type here" << std::endl;
                        return false;
                    }
                    geom_type = mapnik::util::get<mapnik::geometry::geometry_types>(geom_type_value);
                    if (geom_type == mapnik::geometry::geometry_types::GeometryCollection)
                    {
                        if (verbose) std::clog << "GeometryCollections are not allowed" << std::endl;;
                        return false;
                    }
                }
                else if (key2 == mapnik::json::well_known_names::coordinates)
                {
                    auto const& coordinates_value = std::get<1>(elem2);
                    if (!coordinates_value.is<mapnik::json::positions>())
                    {
                        if (verbose) std::clog << "\"coordinates\": xxx <-- expecting an GeoJSON positions here" << std::endl;
                        return false;
                    }
                    coordinates = &mapnik::util::get<mapnik::json::positions>(coordinates_value);
                }
            }
            if (geom_type == mapnik::geometry::geometry_types::Point)
            {
                // expecting single position
                if (!coordinates->is<mapnik::json::point>())
                {
                    if (verbose) std::clog << "Expecting single position in Point" << std::endl;
                    return false;
                }
            }
            else if (geom_type == mapnik::geometry::geometry_types::LineString)
            {
                // expecting
                if (!coordinates->is<mapnik::json::ring>())
                {
                    if (verbose) std::clog << "Expecting sequence of positions (ring) in LineString" << std::endl;
                    return false;
                }
                else
                {
                    auto const& ring = mapnik::util::get<mapnik::json::ring>(*coordinates);
                    if (ring.size() < 2)
                    {
                        if (verbose) std::clog << "Expecting at least two coordinates in LineString" << std::endl;
                        return false;
                    }
                }
            }
            else if (geom_type == mapnik::geometry::geometry_types::Polygon)
            {
                // expecting
                if (!coordinates->is<mapnik::json::rings>())
                {
                    if (verbose) std::clog << "Expecting an array of rings in Polygon" << std::endl;
                    return false;
                }
                else
                {
                    auto const& rings = mapnik::util::get<mapnik::json::rings>(*coordinates);
                    if (rings.size() < 1)
                    {
                        if (verbose) std::clog << "Expecting at least one ring in Polygon" << std::endl;
                        return false;
                    }
                    for (auto const& ring : rings)
                    {
                        if (ring.size() < 4)
                        {
                            if (verbose) std::clog << "Expecting at least four coordinates in Polygon ring" << std::endl;
                            return false;
                        }
                    }
                }
            }
        }
    }
    return true;
};

using box_type = mapnik::box2d<float>;
using boxes_type = std::vector<std::pair<box_type, std::pair<std::size_t, std::size_t>>>;
using base_iterator_type = char const*;
}

namespace mapnik { namespace json {

template <typename Box>
struct calculate_bounding_box
{
    calculate_bounding_box(Box & box)
        : box_(box) {}

    void operator()(mapnik::json::point const& pt) const
    {
        box_.init(pt.x, pt.y);
    }

    void operator()(mapnik::json::ring const& ring) const
    {
        for (auto const& pt : ring)
        {
            if (!box_.valid()) box_.init(pt.x, pt.y);
            else box_.expand_to_include(pt.x, pt.y);
        }
    }

    void operator()(mapnik::json::rings const& rings) const
    {
        for (auto const& ring : rings)
        {
            operator()(ring);
            break; // consider first ring only
        }
    }

    void operator()(mapnik::json::rings_array const& rings_array) const
    {
        for (auto const& rings : rings_array)
        {
            operator()(rings);
        }
    }

    template <typename T>
    void operator() (T const& ) const {}

    Box & box_;
};

namespace grammar {

namespace x3 = boost::spirit::x3;

using x3::lit;
using x3::omit;
using x3::raw;
using x3::char_;
using x3::eps;

struct feature_callback_tag;

auto on_feature_callback = [] (auto const& ctx)
{
    // call our callback
    x3::get<feature_callback_tag>(ctx)(_attr(ctx));
};

namespace {
auto const& geojson_value = geojson_grammar();
}

// extract bounding box from GeoJSON Feature

struct bracket_tag ;

auto check_brackets = [](auto const& ctx)
{
    _pass(ctx) = (x3::get<bracket_tag>(ctx) > 0);
};

auto open_bracket = [](auto const& ctx)
{
    ++x3::get<bracket_tag>(ctx);
};

auto close_bracket = [](auto const& ctx)
{
    --x3::get<bracket_tag>(ctx);
};

auto assign_range = [](auto const& ctx)
{
    std::get<0>(_val(ctx)) = std::move(_attr(ctx));
};

auto assign_bbox = [](auto const& ctx)
{
    std::get<1>(_val(ctx)) = std::move(_attr(ctx));
};

auto extract_bounding_box = [](auto const& ctx)
{
    mapnik::box2d<float>  bbox;
    calculate_bounding_box<mapnik::box2d<float>> visitor(bbox);
    mapnik::util::apply_visitor(visitor, _attr(ctx));
    _val(ctx) = std::move(bbox);
};

auto const coordinates_rule = x3::rule<struct coordinates_rule_tag, mapnik::box2d<float> > {}
    = lit("\"coordinates\"") >> lit(':') >> positions_rule[extract_bounding_box];

auto const bounding_box = x3::rule<struct bounding_box_rule_tag, std::tuple<boost::iterator_range<base_iterator_type>,mapnik::box2d<float>>> {}
    = raw[lit('{')[open_bracket] >> *(eps[check_brackets] >>
                                  (lit("\"FeatureCollection\"") > eps(false)
                                   |
                                   lit('{')[open_bracket]
                                   |
                                   lit('}')[close_bracket]
                                   |
                                   coordinates_rule[assign_bbox]
                                   |
                                   omit[geojson_string]
                                   |
                                   omit[char_]))][assign_range];


auto const feature = bounding_box[on_feature_callback];

auto const key_value_ = omit[geojson_string] > lit(':') > omit[geojson_value] ;

auto const features = lit("\"features\"")
    > lit(':') > lit('[') > -(omit[feature] % lit(',')) > lit(']');

auto const type = lit("\"type\"") > lit(':') > lit("\"FeatureCollection\"");

auto const feature_collection = x3::rule<struct feature_collection_tag> {}
    = lit('{') > (( type | features | key_value_) % lit(',')) > lit('}');


}}}

namespace {
struct collect_features
{
    collect_features(std::vector<mapnik::json::geojson_value> & values)
        : values_(values) {}
    void operator() (mapnik::json::geojson_value && val) const
    {
        values_.push_back(std::move(val));
    }
    std::vector<mapnik::json::geojson_value> & values_;
};

template <typename Iterator, typename Boxes>
struct extract_positions
{
    extract_positions(Iterator start, Boxes & boxes)
        : start_(start),
          boxes_(boxes) {}

    template <typename T>
    void operator() (T const& val) const
    {
        auto const& r = std::get<0>(val);
        mapnik::box2d<float> const& bbox = std::get<1>(val);
        auto offset = std::distance(start_, r.begin());
        auto size = std::distance(r.begin(), r.end());
        boxes_.emplace_back(std::make_pair(bbox,std::make_pair(offset, size)));
        //boxes_.emplace_back(std::make_tuple(bbox,offset, size));
    }
    Iterator start_;
    Boxes & boxes_;
};

}

namespace mapnik { namespace detail {

template <typename T>
std::pair<bool,typename T::value_type::first_type> process_geojson_file_x3(T & boxes, std::string const& filename, bool validate_features, bool verbose)
{
    using box_type = typename T::value_type::first_type;
    box_type extent;
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
    mapnik::mapped_region_ptr mapped_region;
    boost::optional<mapnik::mapped_region_ptr> memory =
        mapnik::mapped_memory_cache::instance().find(filename, true);
    if (!memory)
    {
        std::clog << "Error : cannot memory map " << filename << std::endl;
        return std::make_pair(false, extent);
    }
    else
    {
        mapped_region = *memory;
    }
    base_iterator_type start = reinterpret_cast<base_iterator_type>(mapped_region->get_address());
    base_iterator_type end = start + mapped_region->get_size();
#else
    mapnik::util::file file(filename);
    if (!file)
    {
        std::clog << "Error : cannot open " << filename << std::endl;
        return std::make_pair(false, extent);
    }
    std::string file_buffer;
    file_buffer.resize(file.size());
    std::fread(&file_buffer[0], file.size(), 1, file.get());
    base_iterator_type start = file_buffer.c_str();
    base_iterator_type end = start + file_buffer.length();
#endif
    using namespace boost::spirit;
    using space_type = mapnik::json::grammar::space_type;
    auto const* itr = start;

    extract_positions<base_iterator_type, boxes_type> callback(itr, boxes);
    auto keys = mapnik::json::get_keys();
    std::size_t bracket_counter = 0;
    auto feature_collection_impl = x3::with<mapnik::json::grammar::bracket_tag>(std::ref(bracket_counter))
        [x3::with<mapnik::json::keys_tag>(std::ref(keys))
         [x3::with<mapnik::json::grammar::feature_callback_tag>(std::ref(callback))
          [mapnik::json::grammar::feature_collection]
             ]];

    try
    {
        bool result = x3::phrase_parse(itr, end, feature_collection_impl, space_type());
        if (!result)
        {
            std::clog << "mapnik-index (GeoJSON) : could not extract bounding boxes from : '" <<  filename <<  "'" << std::endl;
            return std::make_pair(false, extent);
        }
    }
    catch (x3::expectation_failure<base_iterator_type> const& ex)
    {
        std::clog << ex.what() << std::endl;
        std::clog << "Expected: " << ex.which();
        std::clog << " Got: \"" << std::string(ex.where(), ex.where() + 200) << '"' << std::endl;
        return std::make_pair(false, extent);
    }
    catch (std::exception const& ex)
    {
        std::clog << "Exception caught:" << ex.what() <<  std::endl;
        return std::make_pair(false, extent);
    }

    auto feature_grammar = x3::with<mapnik::json::keys_tag>(std::ref(keys))
        [ mapnik::json::grammar::geojson_value ];
    for (auto const& item : boxes)
    {
        if (item.first.valid())
        {
            if (!extent.valid()) extent = item.first;
            else extent.expand_to_include(item.first);

            if (validate_features)
            {
                base_iterator_type feat_itr = start + item.second.first;
                base_iterator_type feat_end = feat_itr + item.second.second;
                mapnik::json::geojson_value feature_value;
                try
                {
                    bool result = x3::phrase_parse(feat_itr, feat_end, feature_grammar, space_type(), feature_value);
                    if (!result || feat_itr != feat_end)
                    {
                        if (verbose) std::clog << std::string(start + item.second.first, feat_end ) << std::endl;
                        return std::make_pair(false, extent);
                    }
                }
                catch (x3::expectation_failure<std::string::const_iterator> const& ex)
                {
                    if (verbose) std::clog << ex.what() << std::endl;
                    return std::make_pair(false, extent);
                }
                catch (...)
                {
                    return std::make_pair(false, extent);
                }
                if (!validate_geojson_feature(feature_value, keys, verbose))
                {
                    return std::make_pair(false, extent);
                }
            }
        }
    }
    return std::make_pair(true, extent);
}

template std::pair<bool,box_type> process_geojson_file_x3(boxes_type&, std::string const&, bool, bool);

}}
