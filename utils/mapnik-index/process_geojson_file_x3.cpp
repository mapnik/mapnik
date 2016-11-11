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

template <typename T>
struct feature_validate_callback
{
    feature_validate_callback(mapnik::box2d<T> const& box)
        : box_(box) {}

    void operator() (mapnik::feature_ptr const& f) const
    {
        if (box_ != box_)
        {
            throw std::runtime_error("Bounding boxes mismatch validation feature");
        }
    }
    mapnik::box2d<T> const& box_;
};

using box_type = mapnik::box2d<float>;
using boxes_type = std::vector<std::pair<box_type, std::pair<std::size_t, std::size_t>>>;
//using base_iterator_type = char const*;
//const mapnik::json::extract_bounding_box_grammar<base_iterator_type, boxes_type> geojson_datasource_static_bbox_grammar;
//const mapnik::transcoder tr("utf8");
//const mapnik::json::feature_grammar_callback<base_iterator_type, mapnik::feature_impl, feature_validate_callback<float>> fc_grammar(tr);
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
//auto const& key_value_ = key_value_grammar();
//auto const& json_string = mapnik::json::unicode_string_grammar();
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

auto const bounding_box = x3::rule<struct bounding_box_rule_tag, std::tuple<boost::iterator_range<char const*>,mapnik::box2d<float>>> {}
    = raw[lit('{')[open_bracket] >> *(eps[check_brackets] >>
                                  (lit("\"FeatureCollection\"") > eps(false)
                                   |
                                   lit('{')[open_bracket]
                                   |
                                   lit('}')[close_bracket]
                                   |
                                   coordinates_rule[assign_bbox]
                                   |
                                   omit[json_string]
                                   |
                                   omit[char_]))][assign_range];


auto const feature = bounding_box[on_feature_callback];

auto const key_value_ = omit[json_string] > lit(':') > omit[geojson_value] ;

auto const features = lit("\"features\"")
    > lit(':') > lit('[') > -(omit[feature] % lit(',')) > lit(']');

auto const type = lit("\"type\"") > lit(':') > lit("\"FeatureCollection\"");

auto const feature_collection = x3::rule<struct feature_collection_tag> {}
    = lit('{') > (( type | features | key_value_) % lit(',')) > lit('}');


}}}

namespace {
struct collect_features
{
    collect_features(std::vector<mapnik::json::json_value> & values)
        : values_(values) {}
    void operator() (mapnik::json::json_value && val) const
    {
        values_.push_back(std::move(val));
    }
    std::vector<mapnik::json::json_value> & values_;
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
        //std::clog << offset << ":" << size << " " << bbox << std::endl;
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
    char const* start = reinterpret_cast<char const*>(mapped_region->get_address());
    char const* end = start + mapped_region->get_size();
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
    char const* start = file_buffer.c_str();
    char const* end = start + file_buffer.length();
#endif
    using namespace boost::spirit;
    using space_type = mapnik::json::grammar::space_type;
    auto const* itr = start;

    extract_positions<char const*, boxes_type> callback(itr, boxes);
    mapnik::json::grammar::keys_map keys;
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
    catch (x3::expectation_failure<char const*> const& ex)
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

    mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
    //std::size_t start_id = 1;
    for (auto const& item : boxes)
    {
        if (item.first.valid())
        {
            if (!extent.valid()) extent = item.first;
            else extent.expand_to_include(item.first);

            if (validate_features)
            {
                std::clog << "FIXME" << std::endl;
                return std::make_pair(false, extent);
#if 0
                base_iterator_type feat_itr = start + item.second.first;
                base_iterator_type feat_end = feat_itr + item.second.second;
                feature_validate_callback<float> callback(item.first);
                bool result = boost::spirit::::phrase_parse(feat_itr, feat_end, (fc_grammar)
                                                              (boost::phoenix::ref(ctx), boost::phoenix::ref(start_id), boost::phoenix::ref(callback)),
                                                              space);
                if (!result || feat_itr != feat_end)
                {
                    if (verbose) std::clog << std::string(start + item.second.first, feat_end ) << std::endl;
                    return std::make_pair(false, extent);
                }
#endif
            }
        }
    }
    return std::make_pair(true, extent);
}

template std::pair<bool,box_type> process_geojson_file_x3(boxes_type&, std::string const&, bool, bool);

}}
