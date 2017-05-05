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
#include "csv_inline_featureset.hpp"
#include <mapnik/debug.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/util/utf_conv_win.hpp>
#include <mapnik/util/trim.hpp>
// stl
#include <string>
#include <vector>
#include <deque>

csv_inline_featureset::csv_inline_featureset(std::string const& inline_string,
                                             locator_type const& locator,
                                             char separator,
                                             char quote,
                                             std::vector<std::string> const& headers,
                                             mapnik::context_ptr const& ctx,
                                             array_type && index_array)
    : inline_string_(inline_string),
      separator_(separator),
      quote_(quote),
      headers_(headers),
      index_array_(std::move(index_array)),
      index_itr_(index_array_.begin()),
      index_end_(index_array_.end()),
      ctx_(ctx),
      locator_(locator),
      tr_("utf8") {}

csv_inline_featureset::~csv_inline_featureset() {}

mapnik::feature_ptr csv_inline_featureset::parse_feature(std::string const& str)
{
    auto const* start = str.data();
    auto const* end = start + str.size();
    auto values = csv_utils::parse_line(start, end, separator_, quote_, headers_.size());
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

mapnik::feature_ptr csv_inline_featureset::next()
{
    if (index_itr_ != index_end_)
    {
        csv_datasource::item_type const& item = *index_itr_++;
        std::size_t file_offset = item.second.first;
        std::size_t size = item.second.second;
        std::string str = inline_string_.substr(file_offset, size);
        return parse_feature(str);
    }
    return mapnik::feature_ptr();
}
