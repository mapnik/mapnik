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

#include "csv_utils.hpp"
#include "csv_getline.hpp"
#include "csv_datasource.hpp"
#include "csv_featureset.hpp"
#include "csv_inline_featureset.hpp"
#include "csv_index_featureset.hpp"
// boost
#include <boost/algorithm/string.hpp>
// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/util/utf_conv_win.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/memory_featureset.hpp>
#include <mapnik/boolean.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry/boost_adapters.hpp>
#include <mapnik/util/geometry_to_ds_type.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/util/spatial_index.hpp>
#include <mapnik/geom_util.hpp>
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
MAPNIK_DISABLE_WARNING_POP
#include <mapnik/mapped_memory_cache.hpp>
#endif
#include <mapnik/util/mapped_memory_file.hpp>

// stl
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN_IMPL(csv_datasource_plugin, csv_datasource);
DATASOURCE_PLUGIN_EXPORT(csv_datasource_plugin);
DATASOURCE_PLUGIN_EMPTY_AFTER_LOAD(csv_datasource_plugin);
DATASOURCE_PLUGIN_EMPTY_BEFORE_UNLOAD(csv_datasource_plugin);

csv_datasource::csv_datasource(parameters const& params)
    : datasource(params)
    , desc_(csv_datasource::name(), *params.get<std::string>("encoding", "utf-8"))
    , ctx_(std::make_shared<mapnik::context_type>())
    , tree_(nullptr)
{
    row_limit_ = *params.get<mapnik::value_integer>("row_limit", 0);
    manual_headers_ = mapnik::util::trim_copy(*params.get<std::string>("headers", ""));
    strict_ = *params.get<mapnik::boolean_type>("strict", false);

    auto quote_param = params.get<std::string>("quote");
    if (quote_param)
    {
        auto val = mapnik::util::trim_copy(*quote_param);
        if (!val.empty())
            quote_ = val.front(); // we pick pick first non-space char
    }

    auto separator_param = params.get<std::string>("separator");
    if (separator_param)
    {
        auto val = mapnik::util::trim_copy(*separator_param);
        if (!val.empty())
            separator_ = val.front();
    }

    boost::optional<std::string> ext = params.get<std::string>("extent");
    if (ext && !ext->empty())
    {
        extent_initialized_ = extent_.from_string(*ext);
    }

    boost::optional<std::string> inline_string = params.get<std::string>("inline");
    if (inline_string)
    {
        inline_string_ = *inline_string;
    }
    else
    {
        boost::optional<std::string> file = params.get<std::string>("file");
        if (!file)
            throw mapnik::datasource_exception("CSV Plugin: missing <file> parameter");
        boost::optional<std::string> base = params.get<std::string>("base");
        if (base)
            filename_ = *base + "/" + *file;
        else
            filename_ = *file;

        has_disk_index_ = mapnik::util::exists(filename_ + ".index");
    }
    if (!inline_string_.empty())
    {
        std::istringstream in(inline_string_);
        parse_csv(in);
    }
    else
    {
        mapnik::util::mapped_memory_file in_file{filename_};
        parse_csv(in_file.file());

        if (has_disk_index_ && !extent_initialized_)
        {
            // read bounding box from *.index
            using value_type = mapnik::util::index_record;
            std::ifstream index(filename_ + ".index", std::ios::binary);
            if (!index)
                throw mapnik::datasource_exception("CSV Plugin: could not open: '" + filename_ + ".index'");
            auto ext_f = mapnik::util::spatial_index<value_type,
                                                     mapnik::bounding_box_filter<float>,
                                                     std::ifstream,
                                                     mapnik::box2d<float>>::bounding_box(index);
            extent_ = {ext_f.minx(), ext_f.miny(), ext_f.maxx(), ext_f.maxy()};
        }
        // in.close(); no need to call close, rely on dtor
    }
}

csv_datasource::~csv_datasource() {}

void csv_datasource::parse_csv(std::istream& csv_file)
{
    std::vector<item_type> boxes;
    csv_utils::csv_file_parser::parse_csv_and_boxes(csv_file, boxes);

    std::for_each(headers_.begin(), headers_.end(), [&](std::string const& header) { ctx_->push(header); });

    if (!has_disk_index_)
    {
        // bulk insert initialise r-tree
        tree_ = std::make_unique<spatial_index_type>(boxes);
    }
}

void csv_datasource::add_feature(mapnik::value_integer index, mapnik::csv_line const& values)
{
    if (index != 1)
        return;

    for (std::size_t i = 0; i < values.size(); ++i)
    {
        std::string const& header = headers_.at(i);
        std::string value = mapnik::util::trim_copy(values[i]);
        int value_length = value.length();
        if (locator_.index == i && (locator_.type == csv_utils::geometry_column_locator::WKT ||
                                    locator_.type == csv_utils::geometry_column_locator::GEOJSON))
            continue;

        // First we detect likely strings,
        // then try parsing likely numbers,
        // then try converting to bool,
        // finally falling back to string type.

        // An empty string or a string of "null" will be parsed
        // as a string rather than a true null value.
        // Likely strings are either empty values, very long values
        // or values with leading zeros like 001 (which are not safe
        // to assume are numbers)

        bool matched = false;
        bool has_dot = value.find(".") != std::string::npos;
        if (value.empty() || (value_length > 20) || (value_length > 1 && !has_dot && value[0] == '0'))
        {
            matched = true;
            desc_.add_descriptor(mapnik::attribute_descriptor(header, mapnik::String));
        }
        else if (csv_utils::is_likely_number(value))
        {
            bool has_e = value.find("e") != std::string::npos;
            if (has_dot || has_e)
            {
                double float_val = 0.0;
                if (mapnik::util::string2double(value, float_val))
                {
                    matched = true;
                    desc_.add_descriptor(mapnik::attribute_descriptor(header, mapnik::Double));
                }
            }
            else
            {
                mapnik::value_integer int_val = 0;
                if (mapnik::util::string2int(value, int_val))
                {
                    matched = true;
                    desc_.add_descriptor(mapnik::attribute_descriptor(header, mapnik::Integer));
                }
            }
        }
        if (!matched)
        {
            // NOTE: we don't use mapnik::util::string2bool
            // here because we don't want to treat 'on' and 'off'
            // as booleans, only 'true' and 'false'
            if (csv_utils::ignore_case_equal(value, "true") || csv_utils::ignore_case_equal(value, "false"))
            {
                desc_.add_descriptor(mapnik::attribute_descriptor(header, mapnik::Boolean));
            }
            else // fallback to normal string
            {
                desc_.add_descriptor(mapnik::attribute_descriptor(header, mapnik::String));
            }
        }
    }
}

const char* csv_datasource::name()
{
    return "csv";
}

datasource::datasource_t csv_datasource::type() const
{
    return datasource::Vector;
}

mapnik::box2d<double> csv_datasource::envelope() const
{
    return extent_;
}

mapnik::layer_descriptor csv_datasource::get_descriptor() const
{
    return desc_;
}

boost::optional<mapnik::datasource_geometry_t> csv_datasource::get_geometry_type_impl(std::istream& stream) const
{
    boost::optional<mapnik::datasource_geometry_t> result;
    if (tree_)
    {
        int multi_type = 0;
        auto itr = tree_->qbegin(boost::geometry::index::intersects(extent_));
        auto end = tree_->qend();
        for (std::size_t count = 0; itr != end && count < 5; ++itr, ++count)
        {
            csv_datasource::item_type const& item = *itr;
            std::uint64_t file_offset = item.second.first;
            std::uint64_t size = item.second.second;
            stream.seekg(file_offset);
            std::vector<char> record;
            record.resize(size);
            stream.read(record.data(), size);
            std::string str(record.begin(), record.end());
            try
            {
                auto values = csv_utils::parse_line(str, separator_, quote_);
                auto geom = csv_utils::extract_geometry(values, locator_);
                result = mapnik::util::to_ds_type(geom);
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
            } catch (std::exception const& ex)
            {
                if (strict_)
                    throw ex;
                else
                    MAPNIK_LOG_ERROR(csv) << ex.what();
            }
        }
    }
    else
    {
        // try reading *.index
        using value_type = mapnik::util::index_record;
        std::ifstream index(filename_ + ".index", std::ios::binary);
        if (!index)
            throw mapnik::datasource_exception("CSV Plugin: could not open: '" + filename_ + ".index'");
        mapnik::bounding_box_filter<float> filter{
          mapnik::box2d<float>(extent_.minx(), extent_.miny(), extent_.maxx(), extent_.maxy())};
        std::vector<value_type> positions;
        mapnik::util::spatial_index<value_type,
                                    mapnik::bounding_box_filter<float>,
                                    std::ifstream,
                                    mapnik::box2d<float>>::query_first_n(filter, index, positions, 5);
        int multi_type = 0;
        for (auto const& val : positions)
        {
            stream.seekg(val.off);
            std::vector<char> record;
            record.resize(val.size);
            stream.read(record.data(), val.size);
            std::string str(record.begin(), record.end());
            try
            {
                auto values = csv_utils::parse_line(str, separator_, quote_);
                auto geom = csv_utils::extract_geometry(values, locator_);
                result = mapnik::util::to_ds_type(geom);
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
            } catch (std::exception const& ex)
            {
                if (strict_)
                    throw ex;
                else
                    MAPNIK_LOG_ERROR(csv) << ex.what();
            }
        }
    }
    return result;
}

boost::optional<mapnik::datasource_geometry_t> csv_datasource::get_geometry_type() const
{
    if (inline_string_.empty())
    {
#if defined(_WIN32)
        std::ifstream in(mapnik::utf8_to_utf16(filename_), std::ios_base::in | std::ios_base::binary);
#else
        std::ifstream in(filename_.c_str(), std::ios_base::in | std::ios_base::binary);
#endif
        if (!in.is_open())
        {
            throw mapnik::datasource_exception("CSV Plugin: could not open: '" + filename_ + "'");
        }
        return get_geometry_type_impl(in);
    }
    else
    {
        std::stringstream in(inline_string_);
        return get_geometry_type_impl(in);
    }
}

mapnik::featureset_ptr csv_datasource::features(mapnik::query const& q) const
{
    for (auto const& name : q.property_names())
    {
        bool found_name = false;
        for (auto const& header : headers_)
        {
            if (header == name)
            {
                found_name = true;
                break;
            }
        }
        if (!found_name)
        {
            std::ostringstream s;
            s << "CSV Plugin: no attribute '" << name
              << "'. Valid attributes are: " << boost::algorithm::join(headers_, ",") << ".";
            throw mapnik::datasource_exception(s.str());
        }
    }

    mapnik::box2d<double> const& box = q.get_bbox();
    if (extent_.intersects(box))
    {
        if (tree_)
        {
            csv_featureset::array_type index_array;
            tree_->query(boost::geometry::index::intersects(box), std::back_inserter(index_array));
            std::sort(index_array.begin(), index_array.end(), [](item_type const& item0, item_type const& item1) {
                return item0.second.first < item1.second.first;
            });
            if (inline_string_.empty())
            {
                return std::make_shared<csv_featureset>(filename_,
                                                        locator_,
                                                        separator_,
                                                        quote_,
                                                        headers_,
                                                        ctx_,
                                                        std::move(index_array));
            }
            else
            {
                return std::make_shared<csv_inline_featureset>(inline_string_,
                                                               locator_,
                                                               separator_,
                                                               quote_,
                                                               headers_,
                                                               ctx_,
                                                               std::move(index_array));
            }
        }
        else if (has_disk_index_)
        {
            auto const& bbox = q.get_bbox();
            mapnik::bounding_box_filter<float> const filter(
              mapnik::box2d<float>(bbox.minx(), bbox.miny(), bbox.maxx(), bbox.maxy()));
            return std::make_shared<csv_index_featureset>(filename_,
                                                          filter,
                                                          locator_,
                                                          separator_,
                                                          quote_,
                                                          headers_,
                                                          ctx_);
        }
    }
    return mapnik::make_invalid_featureset();
}

mapnik::featureset_ptr csv_datasource::features_at_point(mapnik::coord2d const& pt, double tol) const
{
    mapnik::box2d<double> query_bbox(pt, pt);
    query_bbox.pad(tol);
    mapnik::query q(query_bbox);
    std::vector<mapnik::attribute_descriptor> const& desc = desc_.get_descriptors();
    for (auto const& item : desc)
    {
        q.add_property_name(item.get_name());
    }
    return features(q);
}
