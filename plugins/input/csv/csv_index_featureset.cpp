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
#include "csv_index_featureset.hpp"
#include <mapnik/debug.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/util/utf_conv_win.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/util/spatial_index.hpp>
// stl
#include <string>
#include <vector>
#include <deque>
#include <fstream>

using value_type = std::pair<std::size_t, std::size_t>;
namespace std {
template <typename InputStream>
InputStream & operator>>(InputStream & in, value_type & value)
{
    in.read(reinterpret_cast<char*>(&value), sizeof(value_type));
    return in;
}
}

csv_index_featureset::csv_index_featureset(std::string const& filename_,
                                           mapnik::filter_in_box const& filter,
                                           detail::geometry_column_locator const& locator,
                                           std::string const& separator,
                                           std::vector<std::string> const& headers,
                                           mapnik::context_ptr const& ctx)
    : separator_(separator),
      headers_(headers),
      ctx_(ctx),
      locator_(locator),
      tr_("utf8")
{
    std::string indexname = filename_ + ".index";
    std::ifstream in(indexname.c_str(), std::ios::binary);
    if (!in) throw mapnik::datasource_exception("CSV Plugin: can't open index file " + indexname);


    std::vector<value_type> positions;
    mapnik::util::spatial_index<value_type,
                                mapnik::filter_in_box,
                                std::ifstream>::query(filter, in, positions);

    std::sort(positions.begin(), positions.end(),
              [](value_type const& lhs, value_type const& rhs) { return lhs.first < rhs.first;});
    for (auto const& pos : positions)
    {
        std::cerr << "OFF=" << pos.first << " SIZE=" << pos.second << std::endl;
    }
}

csv_index_featureset::~csv_index_featureset() {}

/*
mapnik::feature_ptr csv_index_featureset::parse_feature(std::string const& str)
{
    auto values = csv_utils::parse_line(str, separator_);
    auto geom = detail::extract_geometry(values, locator_);
    if (!geom.is<mapnik::geometry::geometry_empty>())
    {
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_, ++feature_id_));
        feature->set_geometry(std::move(geom));
        detail::process_properties(*feature, headers_, values, locator_, tr_);
        return feature;
    }
    return mapnik::feature_ptr();
}
*/
mapnik::feature_ptr csv_index_featureset::next()
{
    return mapnik::feature_ptr();
}
