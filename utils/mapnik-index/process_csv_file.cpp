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

#include "process_csv_file.hpp"
#include "../../plugins/input/csv/csv_utils.hpp"
#include <mapnik/geometry_envelope.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#pragma GCC diagnostic pop
#include <mapnik/mapped_memory_cache.hpp>

namespace mapnik { namespace detail {

template <typename T>
std::pair<bool,box2d<double>> process_csv_file(T & boxes, std::string const& filename, std::string const& manual_headers, char separator, char quote)
{
    using file_source_type = boost::interprocess::ibufferstream;
    file_source_type csv_file;
    mapnik::box2d<double> extent;
    mapnik::mapped_region_ptr mapped_region;
    boost::optional<mapnik::mapped_region_ptr> memory =
        mapnik::mapped_memory_cache::instance().find(filename, true);
    if (memory)
    {
        mapped_region = *memory;
        csv_file.buffer(static_cast<char*>(mapped_region->get_address()),mapped_region->get_size());
    }
    else
    {
        std::clog << "Error : cannot mmap " << filename << std::endl;
        return std::make_pair(false, extent);
    }
    auto file_length = ::detail::file_length(csv_file);
    // set back to start
    csv_file.seekg(0, std::ios::beg);
    char newline;
    bool has_newline;
    char detected_quote;
    std::tie(newline, has_newline, detected_quote) = ::detail::autodect_newline_and_quote(csv_file, file_length);
    if (quote == 0) quote = detected_quote;
    // set back to start
    csv_file.seekg(0, std::ios::beg);
    // get first line
    std::string csv_line;
    csv_utils::getline_csv(csv_file, csv_line, newline, quote);
    if (separator == 0) separator = ::detail::detect_separator(csv_line);
    csv_file.seekg(0, std::ios::beg);
    int line_number = 1;
    ::detail::geometry_column_locator locator;
    std::vector<std::string> headers;
    std::clog << "Parsing CSV using SEPARATOR=" << separator << " QUOTE=" << quote << std::endl;
    if (!manual_headers.empty())
    {
        std::size_t index = 0;
        headers = csv_utils::parse_line(manual_headers, separator, quote);
        for (auto const& header : headers)
        {
            ::detail::locate_geometry_column(header, index++, locator);
            headers.push_back(header);
        }
    }
    else // parse first line as headers
    {
        while (csv_utils::getline_csv(csv_file,csv_line,newline, quote))
        {
            try
            {
                headers = csv_utils::parse_line(csv_line, separator, quote);
                // skip blank lines
                if (headers.size() > 0 && headers[0].empty()) ++line_number;
                else
                {
                    std::size_t index = 0;
                    for (auto & header : headers)
                    {
                        if (header.empty())
                        {
                            // create a placeholder for the empty header
                            std::ostringstream s;
                            s << "_" << index;
                            header = s.str();
                        }
                        else
                        {
                            ::detail::locate_geometry_column(header, index, locator);
                        }
                        ++index;
                    }
                    ++line_number;
                    break;
                }
            }
            catch (std::exception const& ex)
            {
                std::string s("CSV index: error parsing headers: ");
                s += ex.what();
                std::clog << s << std::endl;
                return std::make_pair(false, extent);
            }
        }
    }

    if (locator.type == ::detail::geometry_column_locator::UNKNOWN)
    {
        std::clog << "CSV index: could not detect column headers with the name of wkt, geojson, x/y, or "
                  << "latitude/longitude - this is required for reading geometry data" << std::endl;
        return std::make_pair(false, extent);
    }

    std::size_t num_headers = headers.size();
    auto pos = csv_file.tellg();

    // handle rare case of a single line of data and user-provided headers
    // where a lack of a newline will mean that csv_utils::getline_csv returns false
    bool is_first_row = false;
    if (!has_newline)
    {
        csv_file.setstate(std::ios::failbit);
        pos = 0;
        if (!csv_line.empty())
        {
            is_first_row = true;
        }
    }

    while (is_first_row || csv_utils::getline_csv(csv_file, csv_line, newline, quote))
    {
        auto record_offset = pos;
        auto record_size = csv_line.length();
        pos = csv_file.tellg();
        is_first_row = false;
        // skip blank lines
        if (record_size <= 10)
        {
            std::string trimmed = csv_line;
            boost::trim_if(trimmed, boost::algorithm::is_any_of("\",'\r\n "));
            if (trimmed.empty())
            {
                std::clog << "CSV index: empty row encountered at line: " << line_number << std::endl;
                continue;
            }
        }
        try
        {
            auto values = csv_utils::parse_line(csv_line, separator, quote);
            unsigned num_fields = values.size();
            if (num_fields > num_headers || num_fields < num_headers)
            {
                std::ostringstream s;
                s << "CSV Index: # of columns("
                  << num_fields << ") > # of headers("
                  << num_headers << ") parsed for row " << line_number << "\n";
                std::clog << s.str() << std::endl;
                return std::make_pair(false, extent);
            }

            auto geom = ::detail::extract_geometry(values, locator);
            if (!geom.is<mapnik::geometry::geometry_empty>())
            {
                auto box = mapnik::geometry::envelope(geom);
                if (!extent.valid()) extent = box;
                else extent.expand_to_include(box);
                boxes.emplace_back(std::move(box), make_pair(record_offset, record_size));
            }
            else
            {
                std::ostringstream s;
                s << "CSV Index: expected geometry column: could not parse row "
                  << line_number << " "
                  << values[locator.index] << "'";
                std::clog << s.str() << std::endl;;
            }
        }
        catch (std::exception const& ex)
        {
            std::ostringstream s;
            s << "CSV Index: unexpected error parsing line: " << line_number
              << " - found " << headers.size() << " with values like: " << csv_line << "\n"
              << " and got error like: " << ex.what();
            std::clog << s.str() << std::endl;
            return std::make_pair(false, extent);
        }
    }
    return std::make_pair(true, extent);;
}

using box_type = mapnik::box2d<double>;
using item_type = std::pair<box_type, std::pair<std::size_t, std::size_t>>;
using boxes_type = std::vector<item_type>;
template std::pair<bool,box2d<double>> process_csv_file(boxes_type&, std::string const&, std::string const&, char, char);

}}
