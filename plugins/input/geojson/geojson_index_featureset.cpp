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

// mapnik
#include "geojson_index_featureset.hpp"
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/util/utf_conv_win.hpp>
#include <mapnik/util/spatial_index.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/geometry/is_empty.hpp>
#include <mapnik/json/parse_feature.hpp>
#include <mapnik/json/json_grammar_config.hpp>
// stl
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

geojson_index_featureset::geojson_index_featureset(std::string const& filename, mapnik::bounding_box_filter<float> const& filter)
    :
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
    //
#elif defined _WINDOWS
    file_(_wfopen(mapnik::utf8_to_utf16(filename).c_str(), L"rb"), std::fclose),
#else
    file_(std::fopen(filename.c_str(),"rb"), std::fclose),
#endif
    ctx_(std::make_shared<mapnik::context_type>()),
    query_box_(filter.box_)
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
                                mapnik::bounding_box_filter<float>,
                                std::ifstream,
                                mapnik::box2d<float>>::query(filter, index, positions_);

    std::cerr << "#1 Num features:" << positions_.size() << std::endl;
    positions_.erase(std::remove_if(positions_.begin(),
                                    positions_.end(),
                                    [&](value_type const& pos)
                                    { return !mapnik::box2d<float>{pos.box[0], pos.box[1], pos.box[2], pos.box[3]}.intersects(query_box_);}),
                     positions_.end());
    std::cerr << "#2 Num features:" << positions_.size() << std::endl;

    std::sort(positions_.begin(), positions_.end(),
              [](value_type const& lhs, value_type const& rhs) { return lhs.off < rhs.off;});
    itr_ = positions_.begin();
}

geojson_index_featureset::~geojson_index_featureset() {}

mapnik::feature_ptr geojson_index_featureset::next()
{
    while( itr_ != positions_.end())
    {
        auto pos = *itr_++;
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
        char const* start = (char const*)mapped_region_->get_address() + pos.off;
        char const*  end = start + pos.size;
#else
        std::fseek(file_.get(), pos.off, SEEK_SET);
        std::vector<char> record;
        record.resize(pos.second);
        auto count = std::fread(record.data(), pos.size, 1, file_.get());
        auto const* start = record.data();
        auto const*  end = (count == 1) ? start + record.size() : start;
#endif
        static const mapnik::transcoder tr("utf8");
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_, feature_id_++));
        using mapnik::json::grammar::iterator_type;
        mapnik::json::parse_feature(start, end, *feature, tr); // throw on failure
        // skip empty geometries
        if (mapnik::geometry::is_empty(feature->get_geometry())) continue;
        return feature;
    }
    return mapnik::feature_ptr();
}
