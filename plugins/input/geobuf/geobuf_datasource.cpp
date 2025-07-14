/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#include "geobuf_datasource.hpp"
#include "geobuf_featureset.hpp"
#include "geobuf.hpp"

#include <fstream>
#include <algorithm>
#include <functional>

// boost

#include <boost/algorithm/string.hpp>

// mapnik
#include <mapnik/unicode.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_kv_iterator.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/util/geometry_to_ds_type.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/util/file_io.hpp>
#include <mapnik/geometry/boost_adapters.hpp>

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN_IMPL(geobuf_datasource_plugin, geobuf_datasource);
DATASOURCE_PLUGIN_EXPORT(geobuf_datasource_plugin);
DATASOURCE_PLUGIN_EMPTY_AFTER_LOAD(geobuf_datasource_plugin);
DATASOURCE_PLUGIN_EMPTY_BEFORE_UNLOAD(geobuf_datasource_plugin);

struct attr_value_converter
{
    mapnik::eAttributeType operator()(mapnik::value_integer) const { return mapnik::Integer; }

    mapnik::eAttributeType operator()(double) const { return mapnik::Double; }

    mapnik::eAttributeType operator()(float) const { return mapnik::Double; }

    mapnik::eAttributeType operator()(bool) const { return mapnik::Boolean; }

    mapnik::eAttributeType operator()(std::string const&) const { return mapnik::String; }

    mapnik::eAttributeType operator()(mapnik::value_unicode_string const&) const { return mapnik::String; }

    mapnik::eAttributeType operator()(mapnik::value_null const&) const { return mapnik::String; }
};

geobuf_datasource::geobuf_datasource(parameters const& params)
    : datasource(params),
      type_(datasource::Vector),
      desc_(geobuf_datasource::name(), *params.get<std::string>("encoding", "utf-8")),
      filename_(),
      extent_(),
      features_(),
      tree_(nullptr)
{
    const auto file = params.get<std::string>("file");
    if (!file.has_value())
        throw mapnik::datasource_exception("Geobuf Plugin: missing <file> parameter");

    const auto base = params.get<std::string>("base");
    if (base.has_value())
        filename_ = *base + "/" + *file;
    else
        filename_ = *file;

    mapnik::util::file in(filename_);
    if (!in.is_open())
    {
        throw mapnik::datasource_exception("Geobuf Plugin: could not open: '" + filename_ + "'");
    }
    std::vector<char> geobuf;
    geobuf.resize(in.size());
    std::fread(geobuf.data(), in.size(), 1, in.get());
    parse_geobuf(geobuf.data(), geobuf.size());
}

namespace {
template<typename T>
struct push_feature
{
    using features_container = T;
    push_feature(features_container& features)
        : features_(features)
    {}

    void operator()(mapnik::feature_ptr const& feature) { features_.push_back(feature); }
    features_container& features_;
};
} // namespace

void geobuf_datasource::parse_geobuf(char const* data, std::size_t size)
{
    using push_feature_callback = push_feature<std::vector<mapnik::feature_ptr>>;
    push_feature_callback callback(features_);
    mapnik::util::geobuf<push_feature_callback> buf(data, size, callback);
    buf.read();
    using values_container = std::vector<std::pair<box_type, std::pair<std::size_t, std::size_t>>>;
    values_container values;
    values.reserve(features_.size());
    tree_ = std::make_unique<spatial_index_type>(values);
    std::size_t geometry_index = 0;
    for (mapnik::feature_ptr const& f : features_)
    {
        mapnik::box2d<double> box = f->envelope();
        if (box.valid())
        {
            if (geometry_index == 0)
            {
                extent_ = box;
                for (auto const& kv : *f)
                {
                    desc_.add_descriptor(mapnik::attribute_descriptor(
                      std::get<0>(kv),
                      mapnik::util::apply_visitor(attr_value_converter(), std::get<1>(kv))));
                }
            }
            else
            {
                extent_.expand_to_include(box);
            }
            values.emplace_back(box, std::make_pair(geometry_index, 0));
        }
        ++geometry_index;
    }
    // packing algorithm
    tree_ = std::make_unique<spatial_index_type>(values);
}

geobuf_datasource::~geobuf_datasource() {}

const char* geobuf_datasource::name()
{
    return "geobuf";
}

std::optional<mapnik::datasource_geometry_t> geobuf_datasource::get_geometry_type() const
{
    std::optional<mapnik::datasource_geometry_t> result;
    int multi_type = 0;
    unsigned num_features = features_.size();
    for (unsigned i = 0; i < num_features && i < 5; ++i)
    {
        result = mapnik::util::to_ds_type(features_[i]->get_geometry());
        if (result)
        {
            int type = static_cast<int>(*result);
            if (multi_type > 0 && multi_type != type)
            {
                result = mapnik::datasource_geometry_t::Collection;
                return result;
            }
            multi_type = type;
        }
    }
    return result;
}

mapnik::datasource::datasource_t geobuf_datasource::type() const
{
    return type_;
}

mapnik::box2d<double> geobuf_datasource::envelope() const
{
    return extent_;
}

mapnik::layer_descriptor geobuf_datasource::get_descriptor() const
{
    return desc_;
}

mapnik::featureset_ptr geobuf_datasource::features(mapnik::query const& q) const
{
    // if the query box intersects our world extent then query for features
    mapnik::box2d<double> const& box = q.get_bbox();
    if (extent_.intersects(box))
    {
        geobuf_featureset::array_type index_array;
        if (tree_)
        {
            tree_->query(boost::geometry::index::intersects(box), std::back_inserter(index_array));
            return std::make_shared<geobuf_featureset>(features_, std::move(index_array));
        }
    }
    return mapnik::featureset_ptr();
}

mapnik::featureset_ptr geobuf_datasource::features_at_point(mapnik::coord2d const& pt, double tol) const
{
    mapnik::box2d<double> query_bbox(pt, pt);
    query_bbox.pad(tol);
    mapnik::query q(query_bbox);
    std::vector<mapnik::attribute_descriptor> const& desc = desc_.get_descriptors();
    std::vector<mapnik::attribute_descriptor>::const_iterator itr = desc.begin();
    std::vector<mapnik::attribute_descriptor>::const_iterator end = desc.end();
    for (; itr != end; ++itr)
    {
        q.add_property_name(itr->get_name());
    }
    return features(q);
}
