/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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
#include "csv_index_featureset.hpp"
#include <mapnik/debug.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/util/utf_conv_win.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/geometry.hpp>
// stl
#include <string>
#include <vector>
#include <deque>
#include <fstream>

csv_index_featureset::csv_index_featureset(std::string const& filename,
                                           mapnik::bounding_box_filter<float> const& filter,
                                           locator_type const& locator,
                                           char separator,
                                           char quote,
                                           std::vector<std::string> const& headers,
                                           mapnik::context_ptr const& ctx)
    : separator_(separator)
    , quote_(quote)
    , headers_(headers)
    , ctx_(ctx)
    , locator_(locator)
    , tr_("utf8")
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
//
#elif defined(_WIN32)
    , file_(_wfopen(mapnik::utf8_to_utf16(filename).c_str(), L"rb"), std::fclose)
#else
    , file_(std::fopen(filename.c_str(), "rb"), std::fclose)
#endif

{
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
    const auto memory = mapnik::mapped_memory_cache::instance().find(filename, true);
    if (memory.has_value())
    {
        mapped_region_ = *memory;
    }
    else
    {
        throw std::runtime_error("could not create file mapping for " + filename);
    }
#else
    if (!file_)
        throw mapnik::datasource_exception("CSV Plugin: can't open file " + filename);
#endif

    std::string indexname = filename + ".index";
    std::ifstream index(indexname.c_str(), std::ios::binary);
    if (!index)
        throw mapnik::datasource_exception("CSV Plugin: can't open index file " + indexname);
    mapnik::util::spatial_index<value_type, mapnik::bounding_box_filter<float>, std::ifstream, mapnik::box2d<float>>::
      query(filter, index, positions_);
    positions_.erase(std::remove_if(positions_.begin(),
                                    positions_.end(),
                                    [&](value_type const& pos) { return !pos.box.intersects(filter.box_); }),
                     positions_.end());
    std::sort(positions_.begin(), positions_.end(), [](value_type const& lhs, value_type const& rhs) {
        return lhs.off < rhs.off;
    });
    itr_ = positions_.begin();
}

csv_index_featureset::~csv_index_featureset() {}

mapnik::feature_ptr csv_index_featureset::parse_feature(char const* beg, char const* end)
{
    auto values = csv_utils::parse_line(beg, end, separator_, quote_, headers_.size());
    auto geom = csv_utils::extract_geometry(values, locator_);
    if (!geom.is<mapnik::geometry::geometry_empty>())
    {
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_, ++feature_id_));
        feature->set_geometry(std::move(geom));
        csv_utils::process_properties(*feature, headers_, values, locator_, tr_);
        return feature;
    }
    return mapnik::feature_ptr();
}

mapnik::feature_ptr csv_index_featureset::next()
{
    /*
    if (row_limit_ && count_ >= row_limit_)
    {
        return feature_ptr();
    }
    */

    while (itr_ != positions_.end())
    {
        auto pos = *itr_++;
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
        char const* start = static_cast<char const*>(mapped_region_->get_address()) + pos.off;
        char const* end = start + pos.size;
#else
        std::fseek(file_.get(), pos.off, SEEK_SET);
        std::vector<char> record;
        record.resize(pos.size);
        if (std::fread(record.data(), pos.size, 1, file_.get()) != 1)
        {
            return mapnik::feature_ptr();
        }
        auto const* start = record.data();
        auto const* end = start + record.size();
#endif
        auto feature = parse_feature(start, end);
        if (feature)
            return feature;
    }
    return mapnik::feature_ptr();
}
