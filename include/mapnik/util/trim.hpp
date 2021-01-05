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

#ifndef MAPNIK_TRIM_HPP
#define MAPNIK_TRIM_HPP

// stl
#include <string>
#include <algorithm>

namespace mapnik { namespace util {

/*
   https://github.com/mapnik/mapnik/issues/1633
   faster trim (than boost::trim)
   that intentionally does not respect
   std::locale to avoid overhead in cases
   where the locale is not critical
*/

static inline bool not_whitespace(int ch)
{
    if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') return false;
    return true;
}

// trim from start
static inline std::string & ltrim(std::string & s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_whitespace));
    return s;
}

// trim from end
static inline std::string & rtrim(std::string & s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), not_whitespace).base(), s.end());
    return s;
}

// trim from both ends
static inline void trim(std::string & s)
{
    ltrim(rtrim(s));
}

static inline std::string trim_copy(std::string s)
{
    return ltrim(rtrim(s));
}

static inline bool not_double_quote(int ch)
{
   if (ch == '"') return false;
   return true;
}

static inline void unquote_double(std::string & s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_double_quote));
    s.erase(std::find_if(s.rbegin(), s.rend(), not_double_quote).base(), s.end());
}

static inline bool not_quoted(int ch)
{
   if (ch == '"' || ch == '\'') return false;
   return true;
}

static inline void unquote(std::string & s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_quoted));
    s.erase(std::find_if(s.rbegin(), s.rend(), not_quoted).base(), s.end());
}

}} // end of namespace mapnik

#endif // MAPNIK_TRIM_HPP
