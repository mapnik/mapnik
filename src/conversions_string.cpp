/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#include <mapnik/util/conversions.hpp>
#include <mapnik/value/types.hpp>

#include <algorithm>
#include <cstring>

// karma is used by default
#define MAPNIK_KARMA_TO_STRING

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#ifdef MAPNIK_KARMA_TO_STRING
#include <boost/spirit/include/karma.hpp>
#endif
MAPNIK_DISABLE_WARNING_POP

#if _MSC_VER
#define snprintf _snprintf
#endif

namespace mapnik {
namespace util {

using namespace boost::spirit;

// double conversion - here we use sprintf over karma to work
// around https://github.com/mapnik/mapnik/issues/1741
bool to_string(std::string& s, double val)
{
    s.resize(s.capacity());
    while (true)
    {
        size_t n2 = static_cast<size_t>(snprintf(&s[0], s.size() + 1, "%g", val));
        if (n2 <= s.size())
        {
            s.resize(n2);
            break;
        }
        s.resize(n2);
    }
    return true;
}

#ifdef MAPNIK_KARMA_TO_STRING

bool to_string(std::string& str, int value)
{
    namespace karma = boost::spirit::karma;
    std::back_insert_iterator<std::string> sink(str);
    return karma::generate(sink, value);
}

#ifdef BIGINT
bool to_string(std::string& str, mapnik::value_integer value)
{
    namespace karma = boost::spirit::karma;
    std::back_insert_iterator<std::string> sink(str);
    return karma::generate(sink, value);
}
#endif

bool to_string(std::string& str, unsigned value)
{
    namespace karma = boost::spirit::karma;
    std::back_insert_iterator<std::string> sink(str);
    return karma::generate(sink, value);
}

bool to_string(std::string& str, bool value)
{
    namespace karma = boost::spirit::karma;
    std::back_insert_iterator<std::string> sink(str);
    return karma::generate(sink, value);
}

#else

bool to_string(std::string& s, int val)
{
    s.resize(s.capacity());
    while (true)
    {
        size_t n2 = static_cast<size_t>(snprintf(&s[0], s.size() + 1, "%d", val));
        if (n2 <= s.size())
        {
            s.resize(n2);
            break;
        }
        s.resize(n2);
    }
    return true;
}

#ifdef BIGINT
bool to_string(std::string& s, mapnik::value_integer val)
{
    s.resize(s.capacity());
    while (true)
    {
        size_t n2 = static_cast<size_t>(snprintf(&s[0], s.size() + 1, "%lld", val));
        if (n2 <= s.size())
        {
            s.resize(n2);
            break;
        }
        s.resize(n2);
    }
    return true;
}
#endif

bool to_string(std::string& s, unsigned val)
{
    s.resize(s.capacity());
    while (true)
    {
        size_t n2 = static_cast<size_t>(snprintf(&s[0], s.size() + 1, "%u", val));
        if (n2 <= s.size())
        {
            s.resize(n2);
            break;
        }
        s.resize(n2);
    }
    return true;
}

bool to_string(std::string& s, bool val)
{
    if (val)
        s = "true";
    else
        s = "false";
    return true;
}

#endif

} // end namespace util
} // namespace mapnik
