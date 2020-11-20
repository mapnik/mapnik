/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
MAPNIK_DISABLE_WARNING_POP

namespace mapnik { namespace util {

using namespace boost::spirit;

auto INTEGER = x3::int_type();
#ifdef BIGINT
//auto LONGLONG = x3::long_long_type();
#endif
auto FLOAT = x3::float_type();
auto DOUBLE = x3::double_type();

bool string2bool(std::string const& value, bool& result)
{
    if (value.empty() || value.size() > 5)
    {
        return false;
    }
    else if (value == "true")
    {
        return result = true;
    }
    else if (value == "false")
    {
        result = false;
        return true;
    }
    std::string val(value);
    std::transform(val.begin(), val.end(), val.begin(), ::tolower);
    if (val == "true" || val == "yes" || val == "1" || val == "on")
    {
        return result = true;
    }
    else if (val == "false" || val == "no" || val == "0" || val == "off")
    {
        result = false;
        return true;
    }
    return false;
}

bool string2bool(const char* iter, const char* end, bool& result)
{
    std::string val(iter, end);
    return string2bool(val, result);
}

bool string2int(const char* iter, const char* end, int& result)
{
    x3::ascii::space_type space;
    bool r = x3::phrase_parse(iter, end, INTEGER, space, result);
    return r && (iter == end);
}

bool string2int(std::string const& value, int& result)
{
    x3::ascii::space_type space;
    std::string::const_iterator str_beg = value.begin();
    std::string::const_iterator str_end = value.end();
    bool r = x3::phrase_parse(str_beg, str_end, INTEGER, space, result);
    return r && (str_beg == str_end);
}

#ifdef BIGINT
bool string2int(const char* iter, const char* end, mapnik::value_integer& result)
{
    x3::ascii::space_type space;
    bool r = x3::phrase_parse(iter, end, x3::long_long, space, result);
    return r && (iter == end);
}

bool string2int(std::string const& value, mapnik::value_integer& result)
{
    x3::ascii::space_type space;
    std::string::const_iterator str_beg = value.begin();
    std::string::const_iterator str_end = value.end();
    bool r = x3::phrase_parse(str_beg, str_end, x3::long_long, space, result);
    return r && (str_beg == str_end);
}
#endif

bool string2double(std::string const& value, double& result)
{
    x3::ascii::space_type space;
    std::string::const_iterator str_beg = value.begin();
    std::string::const_iterator str_end = value.end();
    bool r = x3::phrase_parse(str_beg, str_end, DOUBLE, space, result);
    return r && (str_beg == str_end);
}

bool string2double(const char* iter, const char* end, double& result)
{
    x3::ascii::space_type space;
    bool r = x3::phrase_parse(iter, end, DOUBLE, space, result);
    return r && (iter == end);
}

bool string2float(std::string const& value, float& result)
{
    x3::ascii::space_type space;
    std::string::const_iterator str_beg = value.begin();
    std::string::const_iterator str_end = value.end();
    bool r = x3::phrase_parse(str_beg, str_end, FLOAT, space, result);
    return r && (str_beg == str_end);
}

bool string2float(const char* iter, const char* end, float& result)
{
    x3::ascii::space_type space;
    bool r = x3::phrase_parse(iter, end, FLOAT, space, result);
    return r && (iter == end);
}
} // util
} // mapnik
