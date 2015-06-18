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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include <boost/algorithm/string.hpp>
#pragma GCC diagnostic pop

#include <string>
#include <cstdio>

namespace csv_utils
{
    static inline bool is_likely_number(std::string const& value)
    {
        return( strspn( value.c_str(), "e-.+0123456789" ) == value.size() );
    }

    static inline void fix_json_quoting(std::string & csv_line)
    {
        std::string wrapping_char;
        std::string::size_type j_idx = std::string::npos;
        std::string::size_type post_idx = std::string::npos;
        std::string::size_type j_idx_double = csv_line.find("\"{");
        std::string::size_type j_idx_single = csv_line.find("'{");
        if (j_idx_double != std::string::npos)
        {
            wrapping_char = "\"";
            j_idx = j_idx_double;
            post_idx = csv_line.find("}\"");

        }
        else if (j_idx_single != std::string::npos)
        {
            wrapping_char = "'";
            j_idx = j_idx_single;
            post_idx = csv_line.find("}'");
        }
        // we are positive it is valid json
        if (!wrapping_char.empty())
        {
            // grab the json chunk
            std::string json_chunk = csv_line.substr(j_idx,post_idx+wrapping_char.size());
            bool does_not_have_escaped_double_quotes = (json_chunk.find("\\\"") == std::string::npos);
            // ignore properly escaped quotes like \" which need no special handling
            if (does_not_have_escaped_double_quotes)
            {
                std::string pre_json = csv_line.substr(0,j_idx);
                std::string post_json = csv_line.substr(post_idx+wrapping_char.size());
                // handle "" in a string wrapped in "
                // http://tools.ietf.org/html/rfc4180#section-2 item 7.
                // e.g. "{""type"":""Point"",""coordinates"":[30.0,10.0]}"
                if (json_chunk.find("\"\"") != std::string::npos)
                {
                    boost::algorithm::replace_all(json_chunk,"\"\"","\\\"");
                    csv_line = pre_json + json_chunk + post_json;
                }
                // handle " in a string wrapped in '
                // e.g. '{"type":"Point","coordinates":[30.0,10.0]}'
                else
                {
                    // escape " because we cannot exchange for single quotes
                    // https://github.com/mapnik/mapnik/issues/1408
                    boost::algorithm::replace_all(json_chunk,"\"","\\\"");
                    boost::algorithm::replace_all(json_chunk,"'","\"");
                    csv_line = pre_json + json_chunk + post_json;
                }
            }
        }
    }
}

#endif // MAPNIK_CSV_UTILS_DATASOURCE_HPP
