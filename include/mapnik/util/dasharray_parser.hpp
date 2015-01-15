/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#ifndef MAPNIK_UTIL_DASHARRAY_PARSER_HPP
#define MAPNIK_UTIL_DASHARRAY_PARSER_HPP

#include <vector>
#include <string>

namespace mapnik { namespace util {

bool parse_dasharray(std::string const& value, std::vector<double>& dasharray);

inline bool add_dashes(std::vector<double> & buf, std::vector<std::pair<double,double> > & dash)
{
    if (buf.empty()) return false;
    size_t size = buf.size();
    if (size % 2 == 1)
    {
        buf.insert(buf.end(),buf.begin(),buf.end());
    }
    std::vector<double>::const_iterator pos = buf.begin();
    while (pos != buf.end())
    {
        if (*pos > 0.0 || *(pos+1) > 0.0) // avoid both dash and gap eq 0.0
        {
            dash.emplace_back(*pos,*(pos + 1));
        }
        pos +=2;
    }
    return !buf.empty();
}

}}

#endif // MAPNIK_UTIL_DASHARRAY_PARSER_HPP
