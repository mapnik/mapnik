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
                                             detail::geometry_column_locator const& locator,
                                             std::string const& separator,
                                             std::vector<std::string> const& headers,
                                             mapnik::context_ptr const& ctx,
                                             array_type && index_array)
    : inline_string_(inline_string),
      separator_(separator),
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
    auto values = csv_utils::parse_line(str, separator_);
    auto val_beg = values.begin();
    auto val_end = values.end();
    auto geom = detail::extract_geometry(values, locator_);
    if (!geom.is<mapnik::geometry::geometry_empty>())
    {
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_, ++feature_id_));
        feature->set_geometry(std::move(geom));
        auto num_headers = headers_.size();
        for (unsigned i = 0; i < num_headers; ++i)
        {
            std::string const& fld_name = headers_.at(i);
            std::string value;
            if (val_beg == val_end)
            {
                feature->put(fld_name,tr_.transcode(value.c_str()));
                continue;
            }
            else
            {
                value = mapnik::util::trim_copy(*val_beg++);
            }
            int value_length = value.length();
            if (locator_.index == i && (locator_.type == detail::geometry_column_locator::WKT
                                        || locator_.type == detail::geometry_column_locator::GEOJSON)  ) continue;
            bool matched = false;
            bool has_dot = value.find(".") != std::string::npos;
            if (value.empty() ||
                (value_length > 20) ||
                (value_length > 1 && !has_dot && value[0] == '0'))
            {
                matched = true;
                feature->put(fld_name,std::move(tr_.transcode(value.c_str())));
            }
            else if (csv_utils::is_likely_number(value))
            {
                bool has_e = value.find("e") != std::string::npos;
                if (has_dot || has_e)
                {
                    double float_val = 0.0;
                    if (mapnik::util::string2double(value,float_val))
                    {
                        matched = true;
                        feature->put(fld_name,float_val);
                    }
                }
                else
                {
                    mapnik::value_integer int_val = 0;
                    if (mapnik::util::string2int(value,int_val))
                    {
                        matched = true;
                        feature->put(fld_name,int_val);
                    }
                }
            }
            if (!matched)
            {
                // NOTE: we don't use mapnik::util::string2bool
                // here because we don't want to treat 'on' and 'off'
                // as booleans, only 'true' and 'false'
                bool bool_val = false;
                std::string lower_val = value;
                std::transform(lower_val.begin(), lower_val.end(), lower_val.begin(), ::tolower);
                if (lower_val == "true")
                {
                    matched = true;
                    bool_val = true;
                }
                else if (lower_val == "false")
                {
                    matched = true;
                    bool_val = false;
                }
                if (matched)
                {
                    feature->put(fld_name,bool_val);
                }
                else
                {
                    // fallback to normal string
                    feature->put(fld_name,std::move(tr_.transcode(value.c_str())));
                }
            }
        }
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
