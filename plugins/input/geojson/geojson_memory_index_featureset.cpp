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

// mapnik
#include "geojson_memory_index_featureset.hpp"

#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/util/utf_conv_win.hpp>
#include <mapnik/geometry/is_empty.hpp>
#include <mapnik/json/parse_feature.hpp>

// stl
#include <string>
#include <vector>

geojson_memory_index_featureset::geojson_memory_index_featureset(std::string const& filename, array_type&& index_array)
    :
#ifdef _WIN32
      file_(_wfopen(mapnik::utf8_to_utf16(filename).c_str(), L"rb"), std::fclose),
#else
      file_(std::fopen(filename.c_str(), "rb"), std::fclose),
#endif
      index_array_(std::move(index_array)),
      index_itr_(index_array_.begin()),
      index_end_(index_array_.end()),
      ctx_(std::make_shared<mapnik::context_type>())
{
    if (!file_)
        throw std::runtime_error("Can't open " + filename);
}

geojson_memory_index_featureset::~geojson_memory_index_featureset() {}

mapnik::feature_ptr geojson_memory_index_featureset::next()
{
    while (index_itr_ != index_end_)
    {
        geojson_datasource::item_type const& item = *index_itr_++;
        std::size_t file_offset = item.second.first;
        std::size_t size = item.second.second;
        std::fseek(file_.get(), file_offset, SEEK_SET);
        std::vector<char> json;
        json.resize(size);
        auto count = std::fread(json.data(), size, 1, file_.get());
        using chr_iterator_type = char const*;
        chr_iterator_type start = json.data();
        chr_iterator_type end = (count == 1) ? start + json.size() : start;
        static const mapnik::transcoder tr("utf8");
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_, feature_id_++));
        mapnik::json::parse_feature(start, end, *feature, tr); // throw on failure
        // skip empty geometries
        if (mapnik::geometry::is_empty(feature->get_geometry()))
            continue;
        return feature;
    }
    return mapnik::feature_ptr();
}
