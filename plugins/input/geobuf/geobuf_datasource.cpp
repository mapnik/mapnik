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
#include <mapnik/utils.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_kv_iterator.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/util/geometry_to_ds_type.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/util/file_io.hpp>
#include <mapnik/make_unique.hpp>

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(geobuf_datasource)

geobuf_datasource::geobuf_datasource(parameters const& params)
  : datasource(params),
    type_(datasource::Vector),
    desc_(geobuf_datasource::name(),
          *params.get<std::string>("encoding","utf-8")),
    filename_(),
    extent_(),
    features_(),
    tree_(nullptr)
{
    boost::optional<std::string> file = params.get<std::string>("file");
    if (!file) throw mapnik::datasource_exception("GeoJSON Plugin: missing <file> parameter");

    boost::optional<std::string> base = params.get<std::string>("base");
    if (base)
        filename_ = *base + "/" + *file;
    else
        filename_ = *file;


    mapnik::util::file in(filename_);
    if (!in.open())
    {
        throw mapnik::datasource_exception("Geobuf Plugin: could not open: '" + filename_ + "'");
    }
    std::vector<std::uint8_t> geobuf;
    geobuf.resize(in.size());
    std::fread(geobuf.data(), in.size(), 1, in.get());
    parse_geobuf(geobuf.data(), geobuf.size());
}

void geobuf_datasource::parse_geobuf(std::uint8_t const* data, std::size_t size)
{
    mapnik::util::geobuf buf(data, size);
    buf.read(features_);
    std::cerr << "Num of features  = " << features_.size() << std::endl;
}

geobuf_datasource::~geobuf_datasource() {}

const char * geobuf_datasource::name()
{
    return "geobuf";
}

boost::optional<mapnik::datasource::geometry_t> geobuf_datasource::get_geometry_type() const
{
    boost::optional<mapnik::datasource::geometry_t> result;
    int multi_type = 0;
    unsigned num_features = features_.size();
    for (unsigned i = 0; i < num_features && i < 5; ++i)
    {
        mapnik::util::to_ds_type(features_[i]->paths(),result);
        if (result)
        {
            int type = static_cast<int>(*result);
            if (multi_type > 0 && multi_type != type)
            {
                result.reset(mapnik::datasource::Collection);
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
    for ( ;itr!=end;++itr)
    {
        q.add_property_name(itr->get_name());
    }
    return features(q);
}
