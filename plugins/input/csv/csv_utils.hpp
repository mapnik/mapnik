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

#ifndef MAPNIK_CSV_UTILS_DATASOURCE_HPP
#define MAPNIK_CSV_UTILS_DATASOURCE_HPP

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_correct.hpp>
#include <mapnik/wkt/wkt_factory.hpp>
#include <mapnik/json/geometry_parser.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/csv/csv_grammar.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/datasource.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string.hpp>
#pragma GCC diagnostic pop

#include <string>
#include <cstdio>
#include <algorithm>

namespace csv_utils
{

static const mapnik::csv_line_grammar<char const*> line_g;
static const mapnik::csv_white_space_skipper skipper{};

template <typename Iterator>
static mapnik::csv_line parse_line(Iterator start, Iterator end, char separator, char quote, std::size_t num_columns)
{
    mapnik::csv_line values;
    if (num_columns > 0) values.reserve(num_columns);
    if (!boost::spirit::qi::phrase_parse(start, end, (line_g)(separator, quote), skipper, values))
    {
        throw mapnik::datasource_exception("Failed to parse CSV line:\n" + std::string(start, end));
    }
    return values;
}

static inline mapnik::csv_line parse_line(std::string const& line_str, char separator, char quote)
{
    auto start = line_str.c_str();
    auto end   = start + line_str.length();
    return parse_line(start, end, separator, quote, 0);
}

static inline bool is_likely_number(std::string const& value)
{
    return (std::strspn( value.c_str(), "e-.+0123456789" ) == value.size());
}

struct ignore_case_equal_pred
{
    bool operator () (unsigned char a, unsigned char b) const
    {
        return std::tolower(a) == std::tolower(b);
    }
};

inline bool ignore_case_equal(std::string const& s0, std::string const& s1)
{
    return std::equal(s0.begin(), s0.end(),
                      s1.begin(), ignore_case_equal_pred());
}

template <class CharT, class Traits, class Allocator>
std::basic_istream<CharT, Traits>& getline_csv(std::istream& is, std::basic_string<CharT,Traits,Allocator>& s, CharT delim, CharT quote)
{
    typename std::basic_string<CharT,Traits,Allocator>::size_type nread = 0;
    typename std::basic_istream<CharT, Traits>::sentry sentry(is, true);
    if (sentry)
    {
        std::basic_streambuf<CharT, Traits>* buf = is.rdbuf();
        s.clear();
        bool has_quote = false;
        while (nread < s.max_size())
        {
            int c1 = buf->sbumpc();
            if (Traits::eq_int_type(c1, Traits::eof()))
            {
                is.setstate(std::ios_base::eofbit);
                break;
            }
            else
            {
                ++nread;
                CharT c = Traits::to_char_type(c1);
                if (Traits::eq(c, quote))
                    has_quote = !has_quote;
                if (!Traits::eq(c, delim) || has_quote)
                    s.push_back(c);
                else
                    break;// Character is extracted but not appended.
            }
        }
    }
    if (nread == 0 || nread >= s.max_size())
        is.setstate(std::ios_base::failbit);

    return is;
}

}


namespace detail {

template <typename T>
std::size_t file_length(T & stream)
{
    stream.seekg(0, std::ios::end);
    return stream.tellg();
}

template <typename T>
std::tuple<char, bool, char, char> autodect_csv_flavour(T & stream, std::size_t file_length)
{
    // autodetect newlines/quotes/separators
    char newline = '\n'; // default
    bool has_newline = false;
    bool has_single_quote = false;
    char quote = '"'; // default
    char separator = ','; // default
    // local counters
    int num_commas = 0;
    int num_tabs = 0;
    int num_pipes = 0;
    int num_semicolons = 0;

    static std::size_t const max_size = 4000;
    std::size_t size = std::min(file_length, max_size);
    std::vector<char> buffer;
    buffer.resize(size);
    stream.read(buffer.data(), size);
    for (auto c : buffer)
    {
        switch (c)
        {
        case '\r':
            newline = '\r';
            has_newline = true;
            break;
        case '\n':
            has_newline = true;
            break;
        case '\'':
            if (!has_single_quote)
            {
                quote = c;
                has_single_quote = true;
            }
            break;
        case ',':
            if (!has_newline) ++num_commas;
            break;
        case '\t':
            if (!has_newline) ++num_tabs;
            break;
        case '|':
            if (!has_newline) ++num_pipes;
            break;
        case ';':
           if (!has_newline) ++num_semicolons;
            break;
        }
    }
    // detect separator
    if (num_tabs > 0 && num_tabs > num_commas)
    {
        separator = '\t';
        MAPNIK_LOG_DEBUG(csv) << "csv_datasource: auto detected tab separator";
    }
    else // pipes/semicolons
    {
        if (num_pipes > num_commas)
        {
            separator = '|';
            MAPNIK_LOG_DEBUG(csv) << "csv_datasource: auto detected '|' separator";
        }
        else  if (num_semicolons > num_commas)
        {
            separator = ';';
            MAPNIK_LOG_DEBUG(csv) << "csv_datasource: auto detected ';' separator";
        }
    }

    if (has_newline && has_single_quote)
    {
        std::istringstream ss(std::string(buffer.begin(), buffer.end()));
        std::size_t num_columns = 0;
        for (std::string line; csv_utils::getline_csv(ss, line, newline, quote); )
        {
            if (size < file_length && ss.eof())
            {
                // we can't be sure last line
                // is not truncated so skip it
                break;
            }
            if (line.size() == 0) continue; // empty lines are not interesting
            auto num_quotes = std::count(line.begin(), line.end(), quote);
            if (num_quotes % 2 != 0)
            {
                quote = '"';
                break;
            }
            auto columns = csv_utils::parse_line(line, separator, quote);
            if (num_columns > 0 && num_columns != columns.size())
            {
                quote = '"';
                break;
            }
            num_columns = columns.size();
        }
    }
    return std::make_tuple(newline, has_newline, separator, quote);
}

struct geometry_column_locator
{
    geometry_column_locator()
        : type(UNKNOWN), index(-1), index2(-1) {}

    enum { UNKNOWN = 0, WKT, GEOJSON, LON_LAT } type;
    std::size_t index;
    std::size_t index2;
};

static inline void locate_geometry_column(std::string const& header, std::size_t index, geometry_column_locator & locator)
{
    std::string lower_val(header);
    std::transform(lower_val.begin(), lower_val.end(), lower_val.begin(), ::tolower);
    if (lower_val == "wkt" || (lower_val.find("geom") != std::string::npos))
    {
        locator.type = geometry_column_locator::WKT;
        locator.index = index;
    }
    else if (lower_val == "geojson")
    {
        locator.type = geometry_column_locator::GEOJSON;
        locator.index = index;
    }
    else if (lower_val == "x" || lower_val == "lon"
             || lower_val == "lng" || lower_val == "long"
             || (lower_val.find("longitude") != std::string::npos))
    {
        locator.index = index;
        locator.type = geometry_column_locator::LON_LAT;
    }

    else if (lower_val == "y"
             || lower_val == "lat"
             || (lower_val.find("latitude") != std::string::npos))
    {
        locator.index2 = index;
        locator.type = geometry_column_locator::LON_LAT;
    }
}

static inline bool valid(geometry_column_locator const& locator, std::size_t max_size)
{
    if (locator.type == geometry_column_locator::UNKNOWN) return false;
    if (locator.index >= max_size) return false;
    if (locator.type == geometry_column_locator::LON_LAT && locator.index2 >= max_size) return false;
    return true;
}

static inline mapnik::geometry::geometry<double> extract_geometry(std::vector<std::string> const& row, geometry_column_locator const& locator)
{
    mapnik::geometry::geometry<double> geom;
    if (locator.type == geometry_column_locator::WKT)
    {
        auto wkt_value = row.at(locator.index);
        if (mapnik::from_wkt(wkt_value, geom))
        {
            // correct orientations ..
            mapnik::geometry::correct(geom);
        }
        else
        {
            throw mapnik::datasource_exception("Failed to parse WKT: '" + wkt_value + "'");
        }
    }
    else if (locator.type == geometry_column_locator::GEOJSON)
    {

        auto json_value = row.at(locator.index);
        if (!mapnik::json::from_geojson(json_value, geom))
        {
            throw mapnik::datasource_exception("Failed to parse GeoJSON: '" + json_value + "'");
        }
    }
    else if (locator.type == geometry_column_locator::LON_LAT)
    {
        double x, y;
        auto long_value = row.at(locator.index);
        auto lat_value = row.at(locator.index2);
        if (!mapnik::util::string2double(long_value,x))
        {
            throw mapnik::datasource_exception("Failed to parse Longitude: '" + long_value + "'");
        }
        if (!mapnik::util::string2double(lat_value,y))
        {
            throw mapnik::datasource_exception("Failed to parse Latitude: '" + lat_value + "'");
        }
        geom = mapnik::geometry::point<double>(x,y);
    }
    return geom;
}

template <typename Feature, typename Headers, typename Values, typename Locator, typename Transcoder>
void process_properties(Feature & feature, Headers const& headers, Values const& values, Locator const& locator, Transcoder const& tr)
{
    auto val_beg = values.begin();
    auto val_end = values.end();
    auto num_headers = headers.size();
    for (std::size_t i = 0; i < num_headers; ++i)
    {
        std::string const& fld_name = headers.at(i);
        if (val_beg == val_end)
        {
            feature.put(fld_name,tr.transcode(""));
            continue;
        }
        std::string value = mapnik::util::trim_copy(*val_beg++);
        int value_length = value.length();

        if (locator.index == i && (locator.type == detail::geometry_column_locator::WKT
                                   || locator.type == detail::geometry_column_locator::GEOJSON)  ) continue;


        bool matched = false;
        bool has_dot = value.find(".") != std::string::npos;
        if (value.empty() ||
            (value_length > 20) ||
            (value_length > 1 && !has_dot && value[0] == '0'))
        {
            matched = true;
            feature.put(fld_name,std::move(tr.transcode(value.c_str())));
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
                    feature.put(fld_name,float_val);
                }
            }
            else
            {
                mapnik::value_integer int_val = 0;
                if (mapnik::util::string2int(value,int_val))
                {
                    matched = true;
                    feature.put(fld_name,int_val);
                }
            }
        }
        if (!matched)
        {
            if (csv_utils::ignore_case_equal(value, "true"))
            {
                feature.put(fld_name, true);
            }
            else if (csv_utils::ignore_case_equal(value, "false"))
            {
                feature.put(fld_name, false);
            }
            else // fallback to string
            {
                feature.put(fld_name,std::move(tr.transcode(value.c_str())));
            }
        }
    }
}


}// ns detail

#endif // MAPNIK_CSV_UTILS_DATASOURCE_HPP
