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

#include "csv_utils.hpp"
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
#include <mapnik/util/geometry_to_ds_type.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/util/spatial_index.hpp>
#include <mapnik/geom_util.hpp>
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#pragma GCC diagnostic pop
#include <mapnik/mapped_memory_cache.hpp>
#endif

// stl
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(csv_datasource)

csv_datasource::csv_datasource(parameters const& params)
: datasource(params),
    desc_(csv_datasource::name(), *params.get<std::string>("encoding", "utf-8")),
    extent_(),
    filename_(),
    row_limit_(*params.get<mapnik::value_integer>("row_limit", 0)),
    inline_string_(),
    separator_(0),
    quote_(0),
    headers_(),
    manual_headers_(mapnik::util::trim_copy(*params.get<std::string>("headers", ""))),
    strict_(*params.get<mapnik::boolean_type>("strict", false)),
    ctx_(std::make_shared<mapnik::context_type>()),
    extent_initialized_(false),
    tree_(nullptr),
    locator_(),
    has_disk_index_(false)
{
    auto quote_param = params.get<std::string>("quote");
    if (quote_param)
    {
        auto val = mapnik::util::trim_copy(*quote_param);
        if (!val.empty()) quote_ = val.front(); // we pick pick first non-space char
    }

    auto separator_param = params.get<std::string>("separator");
    if (separator_param)
    {
        auto val = mapnik::util::trim_copy(*separator_param);
        if (!val.empty()) separator_ = val.front();
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
        if (!file) throw mapnik::datasource_exception("CSV Plugin: missing <file> parameter");
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
#if defined (MAPNIK_MEMORY_MAPPED_FILE)
        using file_source_type = boost::interprocess::ibufferstream;
        file_source_type in;
        mapnik::mapped_region_ptr mapped_region;
        boost::optional<mapnik::mapped_region_ptr> memory =
            mapnik::mapped_memory_cache::instance().find(filename_, true);
        if (memory)
        {
            mapped_region = *memory;
            in.buffer(static_cast<char*>(mapped_region->get_address()),mapped_region->get_size());
        }
        else
        {
            throw std::runtime_error("could not create file mapping for " + filename_);
        }
#elif defined (_WINDOWS)
        std::ifstream in(mapnik::utf8_to_utf16(filename_),std::ios_base::in | std::ios_base::binary);
        if (!in.is_open())
        {
            throw mapnik::datasource_exception("CSV Plugin: could not open: '" + filename_ + "'");
        }
#else
        std::ifstream in(filename_.c_str(),std::ios_base::in | std::ios_base::binary);
        if (!in.is_open())
        {
            throw mapnik::datasource_exception("CSV Plugin: could not open: '" + filename_ + "'");
        }
#endif
        parse_csv(in);

        if (has_disk_index_ && !extent_initialized_)
        {
            // read bounding box from *.index
            using value_type = std::pair<std::size_t, std::size_t>;
            std::ifstream index(filename_ + ".index", std::ios::binary);
            if (!index) throw mapnik::datasource_exception("CSV Plugin: could not open: '" + filename_ + ".index'");
            extent_ = mapnik::util::spatial_index<value_type,
                                                  mapnik::filter_in_box,
                                                  std::ifstream>::bounding_box(index);
        }
        //in.close(); no need to call close, rely on dtor
    }
}

csv_datasource::~csv_datasource() {}

template <typename T>
void csv_datasource::parse_csv(T & stream)
{
    auto file_length = detail::file_length(stream);
    // set back to start
    stream.seekg(0, std::ios::beg);
    char newline;
    bool has_newline;
    char detected_quote;
    char detected_separator;
    std::tie(newline, has_newline, detected_separator, detected_quote) = detail::autodect_csv_flavour(stream, file_length);
    if (quote_ == 0) quote_ = detected_quote;
    if (separator_ == 0) separator_ = detected_separator;

    // set back to start
    MAPNIK_LOG_DEBUG(csv) << "csv_datasource: separator: '" << separator_
                          << "' quote: '" << quote_ << "'";

    // rewind stream
    stream.seekg(0, std::ios::beg);
    //
    std::string csv_line;
    csv_utils::getline_csv(stream, csv_line, newline, quote_);
    stream.seekg(0, std::ios::beg);
    int line_number = 0;
    if (!manual_headers_.empty())
    {
        std::size_t index = 0;
        auto headers = csv_utils::parse_line(manual_headers_, separator_, quote_);
        for (auto const& header : headers)
        {
            detail::locate_geometry_column(header, index++, locator_);
            headers_.push_back(header);
        }
    }
    else // parse first line as headers
    {
        while (csv_utils::getline_csv(stream, csv_line, newline, quote_))
        {
            try
            {
                auto headers = csv_utils::parse_line(csv_line, separator_, quote_);
                // skip blank lines
                std::string val;
                if (headers.size() > 0 && headers[0].empty()) ++line_number;
                else
                {
                    std::size_t index = 0;
                    for (auto const& header : headers)
                    {
                        val = mapnik::util::trim_copy(header);
                        if (val.empty())
                        {
                            if (strict_)
                            {
                                std::ostringstream s;
                                s << "CSV Plugin: expected a column header at line ";
                                s << line_number << ", column " << index;
                                s << " - ensure this row contains valid header fields: '";
                                s << csv_line;
                                throw mapnik::datasource_exception(s.str());
                            }
                            else
                            {
                                // create a placeholder for the empty header
                                std::ostringstream s;
                                s << "_" << index;
                                headers_.push_back(s.str());
                            }
                        }
                        else
                        {
                            detail::locate_geometry_column(val, index, locator_);
                            headers_.push_back(val);
                        }
                        ++index;
                    }
                    ++line_number;
                    break;
                }
            }
            catch (std::exception const& ex)
            {
                std::string s("CSV Plugin: error parsing headers: ");
                s += ex.what();
                throw mapnik::datasource_exception(s);
            }
        }
    }

    std::size_t num_headers = headers_.size();
    if (!detail::valid(locator_, num_headers))
    {
        std::string str("CSV Plugin: could not detect column(s) with the name(s) of wkt, geojson, x/y, or ");
        str += "latitude/longitude in:\n";
        str += csv_line;
        str += "\n - this is required for reading geometry data";
        throw mapnik::datasource_exception(str);
    }

    mapnik::value_integer feature_count = 0;
    bool extent_started = false;

    std::for_each(headers_.begin(), headers_.end(),
                  [ & ](std::string const& header){ ctx_->push(header); });

    mapnik::transcoder tr(desc_.get_encoding());

    auto pos = stream.tellg();
    // handle rare case of a single line of data and user-provided headers
    // where a lack of a newline will mean that csv_utils::getline_csv returns false
    bool is_first_row = false;

    if (!has_newline)
    {
        stream.setstate(std::ios::failbit);
        pos = 0;
        if (!csv_line.empty())
        {
            is_first_row = true;
        }
    }

    std::vector<item_type> boxes;
    while (is_first_row || csv_utils::getline_csv(stream, csv_line, newline, quote_))
    {
        ++line_number;
        if ((row_limit_ > 0) && (line_number > row_limit_))
        {
            MAPNIK_LOG_DEBUG(csv) << "csv_datasource: row limit hit, exiting at feature: " << feature_count;
            break;
        }
        auto record_offset = pos;
        auto record_size = csv_line.length();
        pos = stream.tellg();
        is_first_row = false;

        // skip blank lines
        if (record_size <= 10)
        {
            std::string trimmed = csv_line;
            boost::trim_if(trimmed,boost::algorithm::is_any_of("\",'\r\n "));
            if (trimmed.empty())
            {
                MAPNIK_LOG_DEBUG(csv) << "csv_datasource: empty row encountered at line: " << line_number;
                continue;
            }
        }

        try
        {
            auto const* line_start = csv_line.data();
            auto const* line_end = line_start + csv_line.size();
            auto values = csv_utils::parse_line(line_start, line_end, separator_, quote_, num_headers);
            unsigned num_fields = values.size();
            if (num_fields != num_headers)
            {
                std::ostringstream s;
                s << "CSV Plugin: # of columns(" << num_fields << ")";
                if (num_fields > num_headers)
                {
                    s << " > ";
                }
                else
                {
                    s << " < ";
                }
                s << "# of headers(" << num_headers << ") parsed";
                throw mapnik::datasource_exception(s.str());
            }

            auto geom = detail::extract_geometry(values, locator_);
            if (!geom.is<mapnik::geometry::geometry_empty>())
            {
                auto box = mapnik::geometry::envelope(geom);
                boxes.emplace_back(std::move(box), make_pair(record_offset, record_size));
                if (!extent_initialized_)
                {
                    if (!extent_started)
                    {
                        extent_started = true;
                        extent_ = mapnik::geometry::envelope(geom);
                    }
                    else
                    {
                        extent_.expand_to_include(mapnik::geometry::envelope(geom));
                    }
                }
                if (++feature_count != 1) continue;
                auto beg = values.begin();
                for (std::size_t i = 0; i < num_headers; ++i)
                {
                    std::string const& header = headers_.at(i);
                    std::string value = mapnik::util::trim_copy(*beg++);
                    int value_length = value.length();
                    if (locator_.index == i && (locator_.type == detail::geometry_column_locator::WKT
                                                || locator_.type == detail::geometry_column_locator::GEOJSON)) continue;

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
                            if (mapnik::util::string2double(value,float_val))
                            {
                                matched = true;
                                desc_.add_descriptor(mapnik::attribute_descriptor(header,mapnik::Double));
                            }
                        }
                        else
                        {
                            mapnik::value_integer int_val = 0;
                            if (mapnik::util::string2int(value,int_val))
                            {
                                matched = true;
                                desc_.add_descriptor(mapnik::attribute_descriptor(header,mapnik::Integer));
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
            else
            {
                std::ostringstream s;
                s << "CSV Plugin: expected geometry column: could not parse row "
                  << line_number << " "
                  << values.at(locator_.index) << "'";
                throw mapnik::datasource_exception(s.str());
            }
        }
        catch (mapnik::datasource_exception const& ex )
        {
            if (strict_) throw ex;
            else
            {
                MAPNIK_LOG_ERROR(csv) << ex.what() << " at line: " << line_number;
            }
        }
        catch (std::exception const& ex)
        {
            std::ostringstream s;
            s << "CSV Plugin: unexpected error parsing line: " << line_number
              << " - found " << headers_.size() << " with values like: " << csv_line << "\n"
              << " and got error like: " << ex.what();
            if (strict_)
            {
                throw mapnik::datasource_exception(s.str());
            }
            else
            {
                MAPNIK_LOG_ERROR(csv) << s.str();
            }
        }
        // return early if *.index is present
        if (has_disk_index_) return;
    }
    // bulk insert initialise r-tree
    tree_ = std::make_unique<spatial_index_type>(boxes);
}

const char * csv_datasource::name()
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

template <typename T>
boost::optional<mapnik::datasource_geometry_t> csv_datasource::get_geometry_type_impl(T & stream) const
{
    boost::optional<mapnik::datasource_geometry_t> result;
    if (tree_)
    {
        int multi_type = 0;
        auto itr = tree_->qbegin(boost::geometry::index::intersects(extent_));
        auto end = tree_->qend();
        for (std::size_t count = 0; itr !=end &&  count < 5; ++itr, ++count)
        {
            csv_datasource::item_type const& item = *itr;
            std::size_t file_offset = item.second.first;
            std::size_t size = item.second.second;
            stream.seekg(file_offset);
            std::vector<char> record;
            record.resize(size);
            stream.read(record.data(), size);
            std::string str(record.begin(), record.end());
            try
            {
                auto values = csv_utils::parse_line(str, separator_, quote_);
                auto geom = detail::extract_geometry(values, locator_);
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
            }
            catch (std::exception const& ex)
            {
                if (strict_) throw ex;
                else MAPNIK_LOG_ERROR(csv) << ex.what();
            }
        }
    }
    else
    {
        // try reading *.index
        using value_type = std::pair<std::size_t, std::size_t>;
        std::ifstream index(filename_ + ".index", std::ios::binary);
        if (!index) throw mapnik::datasource_exception("CSV Plugin: could not open: '" + filename_ + ".index'");

        mapnik::filter_in_box filter(extent_);
        std::vector<value_type> positions;
        mapnik::util::spatial_index<value_type,
                                    mapnik::filter_in_box,
                                    std::ifstream>::query_first_n(filter, index, positions, 5);
        int multi_type = 0;
        for (auto const& val : positions)
        {
            stream.seekg(val.first);
            std::vector<char> record;
            record.resize(val.second);
            stream.read(record.data(), val.second);
            std::string str(record.begin(), record.end());
            try
            {
                auto values = csv_utils::parse_line(str, separator_, quote_);
                auto geom = detail::extract_geometry(values, locator_);
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
            }
            catch (std::exception const& ex)
            {
                if (strict_) throw ex;
                else MAPNIK_LOG_ERROR(csv) << ex.what();
            }
        }

    }
    return result;
}

boost::optional<mapnik::datasource_geometry_t> csv_datasource::get_geometry_type() const
{
    if (inline_string_.empty())
    {
#if defined (_WINDOWS)
        std::ifstream in(mapnik::utf8_to_utf16(filename_),std::ios_base::in | std::ios_base::binary);
#else
        std::ifstream in(filename_.c_str(),std::ios_base::in | std::ios_base::binary);
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
            s << "CSV Plugin: no attribute '" << name << "'. Valid attributes are: "
              << boost::algorithm::join(headers_, ",") << ".";
            throw mapnik::datasource_exception(s.str());
        }
    }

    mapnik::box2d<double> const& box = q.get_bbox();
    if (extent_.intersects(box))
    {
        if (tree_)
        {
            csv_featureset::array_type index_array;
            tree_->query(boost::geometry::index::intersects(box),std::back_inserter(index_array));
            std::sort(index_array.begin(),index_array.end(),
                      [] (item_type const& item0, item_type const& item1)
                      {
                          return item0.second.first < item1.second.first;
                      });
            if (inline_string_.empty())
            {
                return std::make_shared<csv_featureset>(filename_, locator_, separator_, quote_, headers_, ctx_, std::move(index_array));
            }
            else
            {
                return std::make_shared<csv_inline_featureset>(inline_string_, locator_, separator_, quote_, headers_, ctx_, std::move(index_array));
            }
        }
        else if (has_disk_index_)
        {
            mapnik::filter_in_box filter(q.get_bbox());
            return std::make_shared<csv_index_featureset>(filename_, filter, locator_, separator_, quote_, headers_, ctx_);
        }
    }
    return mapnik::featureset_ptr();
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
