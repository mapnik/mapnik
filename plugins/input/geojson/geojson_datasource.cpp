/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#include "geojson_datasource.hpp"
#include "geojson_featureset.hpp"
#include "geojson_index_featureset.hpp"
#include "geojson_memory_index_featureset.hpp"
#include <fstream>
#include <algorithm>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string.hpp>
MAPNIK_DISABLE_WARNING_POP

// mapnik
#include <mapnik/boolean.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/util/utf_conv_win.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/feature_kv_iterator.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/util/file_io.hpp>
#include <mapnik/util/geometry_to_ds_type.hpp>
#include <mapnik/geometry/boost_adapters.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/util/spatial_index.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/json/parse_feature.hpp>
#include <mapnik/json/extract_bounding_boxes_x3.hpp>

#if defined(MAPNIK_MEMORY_MAPPED_FILE)
#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/interprocess/mapped_region.hpp>
MAPNIK_DISABLE_WARNING_POP
#include <mapnik/mapped_memory_cache.hpp>
#endif

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN_IMPL(geojson_datasource_plugin, geojson_datasource);
DATASOURCE_PLUGIN_EXPORT(geojson_datasource_plugin);
DATASOURCE_PLUGIN_EMPTY_INIT(geojson_datasource_plugin);

struct attr_value_converter
{
    mapnik::eAttributeType operator() (mapnik::value_integer) const
    {
        return mapnik::Integer;
    }

    mapnik::eAttributeType operator() (double) const
    {
        return mapnik::Double;
    }

    mapnik::eAttributeType operator() (float) const
    {
        return mapnik::Double;
    }

    mapnik::eAttributeType operator() (bool) const
    {
        return mapnik::Boolean;
    }

    mapnik::eAttributeType operator() (std::string const& ) const
    {
        return mapnik::String;
    }

    mapnik::eAttributeType operator() (mapnik::value_unicode_string const&) const
    {
        return mapnik::String;
    }

    mapnik::eAttributeType operator() (mapnik::value_null const& ) const
    {
        return mapnik::String;
    }
};

geojson_datasource::geojson_datasource(parameters const& params)
    : datasource(params),
      type_(datasource::Vector),
      desc_(geojson_datasource::name(),
            *params.get<std::string>("encoding","utf-8")),
      filename_(),
      from_inline_string_(false),
      extent_(),
      features_(),
      tree_(nullptr),
      num_features_to_query_(std::max(mapnik::value_integer(1), *params.get<mapnik::value_integer>("num_features_to_query", 5)))
{
    boost::optional<std::string> inline_string = params.get<std::string>("inline");
    if (!inline_string)
    {
        boost::optional<std::string> file = params.get<std::string>("file");
        if (!file) throw mapnik::datasource_exception("GeoJSON Plugin: missing <file> parameter");

        boost::optional<std::string> base = params.get<std::string>("base");
        if (base)
            filename_ = *base + "/" + *file;
        else
            filename_ = *file;
        has_disk_index_ = mapnik::util::exists(filename_ + ".index");
    }

    if (inline_string)
    {
        from_inline_string_ = true;
        char const* start = inline_string->c_str();
        char const* end = start + inline_string->size();
        parse_geojson(start, end);
    }
    else if (has_disk_index_)
    {
        initialise_disk_index(filename_);
    }
    else
    {
        cache_features_ = *params.get<mapnik::boolean_type>("cache_features", true);
#if !defined(MAPNIK_MEMORY_MAPPED_FILE)
        mapnik::util::file file(filename_);
        if (!file)
        {
            throw mapnik::datasource_exception("GeoJSON Plugin: could not open: '" + filename_ + "'");
        }

        std::string file_buffer;
        file_buffer.resize(file.size());
        auto count = std::fread(&file_buffer[0], file.size(), 1, file.get());
        char const* start = file_buffer.c_str();
        char const* end = (count == 1) ? start + file_buffer.length() : start;
#else
        boost::optional<mapnik::mapped_region_ptr> mapped_region =
            mapnik::mapped_memory_cache::instance().find(filename_, false);
        if (!mapped_region)
        {
            throw std::runtime_error("could not get file mapping for "+ filename_);
        }

        char const* start = reinterpret_cast<char const*>((*mapped_region)->get_address());
        char const* end = start + (*mapped_region)->get_size();
#endif
        if (cache_features_)
        {
            parse_geojson(start, end);
        }
        else
        {
            initialise_index(start, end);
        }
    }
}

namespace {

using box_type = box2d<double>;
using boxes_type = std::vector<std::pair<box_type, std::pair<std::uint64_t, std::uint64_t>>>;
using base_iterator_type = char const*;
const mapnik::transcoder geojson_datasource_static_tr("utf8");

}

void geojson_datasource::initialise_descriptor(mapnik::feature_ptr const& feature)
{
    for ( auto const& kv : *feature)
    {
        auto const& name = std::get<0>(kv);
        if (!desc_.has_name(name))
        {
            desc_.add_descriptor(mapnik::attribute_descriptor(name,
                                                              mapnik::util::apply_visitor(attr_value_converter(),
                                                                                          std::get<1>(kv))));
        }
    }
}

void geojson_datasource::initialise_disk_index(std::string const& filename)
{
    // read extent
    using value_type = mapnik::util::index_record;
    std::ifstream index(filename_ + ".index", std::ios::binary);
    if (!index) throw mapnik::datasource_exception("GeoJSON Plugin: could not open: '" + filename_ + ".index'");
    auto ext_f = mapnik::util::spatial_index<value_type,
                                             mapnik::bounding_box_filter<float>,
                                             std::ifstream,
                                             mapnik::box2d<float>>::bounding_box(index);
    extent_ = { ext_f.minx(), ext_f.miny(),ext_f.maxx(), ext_f.maxy() };
    mapnik::bounding_box_filter<float> filter(ext_f);
    std::vector<value_type> positions;
    mapnik::util::spatial_index<value_type,
                                mapnik::bounding_box_filter<float>,
                                std::ifstream, mapnik::box2d<float>>::query_first_n(filter, index, positions, num_features_to_query_);

    mapnik::util::file file(filename_);
    if (!file) throw mapnik::datasource_exception("GeoJSON Plugin: could not open: '" + filename_ + "'");
    mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
    for (auto const& pos : positions)
    {
        std::fseek(file.get(), pos.off, SEEK_SET);
        std::vector<char> record;
        record.resize(pos.size);
        auto count = std::fread(record.data(), pos.size, 1, file.get());
        auto const* start = record.data();
        auto const*  end = (count == 1) ? start + record.size() : start;
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx, -1));
        try
        {
            mapnik::json::parse_feature(start, end, *feature, geojson_datasource_static_tr);
        }
        catch (...)
        {
            throw std::runtime_error("Failed to parse geojson feature");
        }
        initialise_descriptor(feature);
    }
    desc_.order_by_name();
}

namespace mapnik { namespace json {

template <typename Iterator>
void parse_geometry_or_feature(Iterator& itr, Iterator const& end, feature_impl& feature, transcoder const& tr )
{
    Iterator start = itr;
    try
    {
        parse_feature(start, end, feature, tr);
    }
    catch (...)
    {
        start = itr;
        parse_geometry(start, end, feature);
    }
}

}}


template <typename Iterator>
void geojson_datasource::initialise_index(Iterator start, Iterator end)
{
    boxes_type boxes;
    mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
    Iterator itr = start;
    try
    {
        mapnik::json::extract_bounding_boxes(itr, end, boxes);
        if (itr != end || boxes.empty()) throw std::exception();
        // bulk insert initialise r-tree
        tree_ = std::make_unique<spatial_index_type>(boxes);
        // calculate total extent
        std::size_t feature_count = 0;

        for (auto const& item : boxes)
        {
            auto const& box = std::get<0>(item);
            auto const& geometry_index = std::get<1>(item);
            if (!extent_.valid()) extent_ = box;
            else extent_.expand_to_include(box);
            if (feature_count++ < num_features_to_query_)
            {
                // parse first N features to extract attributes schema.
                // NOTE: this doesn't yield correct answer for geoJSON in general, just an indication
                mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx,-1)); // temp feature
                Iterator itr2 = start + geometry_index.first;
                Iterator end2 = itr2 + geometry_index.second;
                try
                {
                    mapnik::json::parse_feature(itr2, end2, *feature, geojson_datasource_static_tr);
                }
                catch (...)
                {
                    throw std::runtime_error("Failed to parse geojson feature");
                }
                initialise_descriptor(feature);
            }
        }
    }
    catch (...)
    {

        cache_features_ = true; // force caching single feature
        itr = start; // reset iteraror
        std::size_t start_id = 1;
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx, start_id)); // single feature
        try
        {
            mapnik::json::parse_geometry_or_feature(itr, end, *feature, geojson_datasource_static_tr);
        }
        catch (...)
        {
            if (from_inline_string_) throw mapnik::datasource_exception("geojson_datasource: Failed to parse GeoJSON file from in-memory string");
            else throw mapnik::datasource_exception("geojson_datasource: Failed to parse GeoJSON file '" + filename_ + "'");
        }

        features_.push_back(std::move(feature));

        using values_container = std::vector< std::pair<box_type, std::pair<std::uint64_t, std::uint64_t>>>;
        values_container values;
        values.reserve(features_.size());

        std::size_t geometry_index = 0;
        for (mapnik::feature_ptr const& f : features_)
        {
            mapnik::box2d<double> box = f->envelope();
            if (box.valid())
            {
                if (geometry_index == 0)
                {
                    extent_ = box;
                }
                else
                {
                    extent_.expand_to_include(box);
                }
                values.emplace_back(box, std::make_pair(geometry_index,0));

            }
            if (geometry_index++ < num_features_to_query_)
            {
                initialise_descriptor(f);
            }
        }
        // packing algorithm
        tree_ = std::make_unique<spatial_index_type>(values);
    }
    desc_.order_by_name();
}

template <typename Iterator>
void geojson_datasource::parse_geojson(Iterator start, Iterator end)
{
    mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
    std::size_t start_id = 1;
    Iterator itr = start;
    try
    {
        boxes_type boxes;
        mapnik::json::extract_bounding_boxes(itr, end, boxes);
        if (itr != end || boxes.empty()) throw std::exception(); //ensure we've consumed all input and we extracted at least one bbox;
        for (auto const& item : boxes)
        {
            auto const& geometry_index = std::get<1>(item);
            Iterator itr2 = start + geometry_index.first;
            Iterator end2 = itr2 + geometry_index.second;
            mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx, start_id++));
            mapnik::json::parse_feature(itr2, end2, *feature, geojson_datasource_static_tr);
            features_.push_back(std::move(feature));
        }
    }
    catch (...)
    {
        itr = start;
        // try parsing as single Feature or single Geometry JSON
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx, start_id)); // single feature
        try
        {
            mapnik::json::parse_geometry_or_feature(itr, end, *feature, geojson_datasource_static_tr);
        }
        catch (...)
        {
            if (from_inline_string_) throw mapnik::datasource_exception("geojson_datasource: Failed to parse GeoJSON file from in-memory string");
            else throw mapnik::datasource_exception("geojson_datasource: Failed to parse GeoJSON file '" + filename_ + "'");
        }

        features_.push_back(std::move(feature));
    }
    using values_container = std::vector< std::pair<box_type, std::pair<std::uint64_t, std::uint64_t>>>;
    values_container values;
    values.reserve(features_.size());

    std::size_t geometry_index = 0;
    for (mapnik::feature_ptr const& f : features_)
    {
        mapnik::box2d<double> box = f->envelope();
        if (box.valid())
        {
            if (geometry_index == 0)
            {
                extent_ = box;
            }
            else
            {
                extent_.expand_to_include(box);
            }
            values.emplace_back(box, std::make_pair(geometry_index,0));
        }
        if (geometry_index < num_features_to_query_)
        {
            initialise_descriptor(f);
        }
        ++geometry_index;
    }
    // packing algorithm
    tree_ = std::make_unique<spatial_index_type>(values);
}

geojson_datasource::~geojson_datasource() {}

const char * geojson_datasource::name()
{
    return "geojson";
}

mapnik::datasource::datasource_t geojson_datasource::type() const
{
    return type_;
}

mapnik::box2d<double> geojson_datasource::envelope() const
{
    return extent_;
}

mapnik::layer_descriptor geojson_datasource::get_descriptor() const
{
    return desc_;
}

boost::optional<mapnik::datasource_geometry_t> geojson_datasource::get_geometry_type() const
{
    boost::optional<mapnik::datasource_geometry_t> result;
    int multi_type = 0;
    if (has_disk_index_)
    {
        using value_type = mapnik::util::index_record;
        std::ifstream index(filename_ + ".index", std::ios::binary);
        if (!index) throw mapnik::datasource_exception("GeoJSON Plugin: could not open: '" + filename_ + ".index'");
        mapnik::bounding_box_filter<float> filter(mapnik::box2d<float>(extent_.minx(),extent_.miny(), extent_.maxx(),extent_.maxy()));
        std::vector<value_type> positions;
        mapnik::util::spatial_index<value_type,
                                    mapnik::bounding_box_filter<float>,
                                    std::ifstream,
                                    mapnik::box2d<float>>::query_first_n(filter, index, positions, num_features_to_query_);

        mapnik::util::file file(filename_);

        if (!file) throw mapnik::datasource_exception("GeoJSON Plugin: could not open: '" + filename_ + "'");
        mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
        for (auto const& pos : positions)
        {
            std::fseek(file.get(), pos.off, SEEK_SET);
            std::vector<char> record;
            record.resize(pos.size);
            auto count = std::fread(record.data(), pos.size, 1, file.get());
            auto const* start = record.data();
            auto const*  end = (count == 1) ? start + record.size() : start;
            mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx, -1)); // temp feature
            try
            {
                mapnik::json::parse_feature(start, end, *feature, geojson_datasource_static_tr);
            }
            catch (...)
            {
                throw std::runtime_error("Failed to parse geojson feature");
            }
            result = mapnik::util::to_ds_type(feature->get_geometry());
            if (result)
            {
                int type = static_cast<int>(*result);
                if (multi_type > 0 && multi_type != type)
                {
                    result.reset(mapnik::datasource_geometry_t::Collection);
                    return result;
                }
                multi_type = type;
            }
        }
    }
    else if (cache_features_)
    {
        std::size_t num_features = features_.size();
        for (std::size_t i = 0; i < num_features && i < num_features_to_query_; ++i)
        {
            result = mapnik::util::to_ds_type(features_[i]->get_geometry());
            if (result)
            {
                int type = static_cast<int>(*result);
                if (multi_type > 0 && multi_type != type)
                {
                    result.reset(mapnik::datasource_geometry_t::Collection);
                    return result;
                }
                multi_type = type;
            }
        }
    }
    else
    {
        mapnik::util::file file(filename_);
        if (!file)
        {
            throw mapnik::datasource_exception("GeoJSON Plugin: could not open: '" + filename_ + "'");
        }
        auto itr = tree_->qbegin(boost::geometry::index::intersects(extent_));
        auto end = tree_->qend();
        mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
        for (std::size_t count = 0; itr !=end &&  count < num_features_to_query_; ++itr,++count)
        {
            geojson_datasource::item_type const& item = *itr;
            std::uint64_t file_offset = item.second.first;
            std::uint64_t size = item.second.second;

            std::fseek(file.get(), file_offset, SEEK_SET);
            std::vector<char> json;
            json.resize(size);
            auto count_objects = std::fread(json.data(), size, 1, file.get());
            using chr_iterator_type = char const*;
            chr_iterator_type start2 = json.data();
            chr_iterator_type end2 = (count_objects == 1) ? start2 + json.size() : start2;

            mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx, -1)); // temp feature
            try
            {
                mapnik::json::parse_feature(start2, end2, *feature, geojson_datasource_static_tr);
            }
            catch (...)
            {
                throw std::runtime_error("Failed to parse geojson feature");
            }

            result = mapnik::util::to_ds_type(feature->get_geometry());
            if (result)
            {
                int type = static_cast<int>(*result);
                if (multi_type > 0 && multi_type != type)
                {
                    result.reset(mapnik::datasource_geometry_t::Collection);
                    return result;
                }
                multi_type = type;
            }
        }
    }
    return result;
}

mapnik::featureset_ptr geojson_datasource::features(mapnik::query const& q) const
{
    // if the query box intersects our world extent then query for features
    mapnik::box2d<double> const& box = q.get_bbox();
    if (extent_.intersects(box))
    {
        geojson_featureset::array_type index_array;
        if (tree_)
        {
            tree_->query(boost::geometry::index::intersects(box),std::back_inserter(index_array));
            // sort index array to preserve original feature ordering in GeoJSON
            std::sort(index_array.begin(),index_array.end(),
                      [] (item_type const& item0, item_type const& item1)
                      {
                          return item0.second.first < item1.second.first;
                      });
            if (cache_features_)
            {
                return std::make_shared<geojson_featureset>(features_, std::move(index_array));
            }
            else
            {
                return std::make_shared<geojson_memory_index_featureset>(filename_, std::move(index_array));
            }
        }
        else if (has_disk_index_)
        {
            auto const& bbox = q.get_bbox();
            mapnik::bounding_box_filter<float> const filter(mapnik::box2d<float>(bbox.minx(), bbox.miny(), bbox.maxx(), bbox.maxy()));
            return std::make_shared<geojson_index_featureset>(filename_, filter);
        }
    }
    // otherwise return an empty featureset
    return mapnik::make_invalid_featureset();
}

mapnik::featureset_ptr geojson_datasource::features_at_point(mapnik::coord2d const& pt, double tol) const
{
    mapnik::box2d<double> query_bbox(pt, pt);
    query_bbox.pad(tol);
    mapnik::query q(query_bbox);
    for (auto const& attr_info : desc_.get_descriptors())
    {
        q.add_property_name(attr_info.get_name());
    }
    return features(q);
}
