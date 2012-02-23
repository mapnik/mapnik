/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

#ifndef MAPNIK_CONVERSIONS_UTIL_HPP
#define MAPNIK_CONVERSIONS_UTIL_HPP

// mapnik

// boost
#include <boost/spirit/include/qi.hpp>

// stl
#include <string>

namespace mapnik { namespace conversions {

using namespace boost::spirit;

// TODO - convert to templates

static bool string2int(const char * value, int * result)
{
    size_t length = strlen(value);
    if (length < 1 || value == NULL)
        return false;
    const char *begin = value;
    const char *iter  = begin;
    const char *end   = value + length;
    bool r = qi::phrase_parse(iter,end,qi::int_,ascii::space,*result);
    return r && (iter == end);
}

static bool string2int(std::string const& value, int * result)
{
    if (value.empty())
        return false;
    std::string::const_iterator str_beg = value.begin();
    std::string::const_iterator str_end = value.end();
    bool r = qi::phrase_parse(str_beg,str_end,qi::int_,ascii::space,*result);
    return r && (str_beg == str_end);
}

static bool string2double(std::string const& value, double * result)
{
    if (value.empty())
        return false;
    std::string::const_iterator str_beg = value.begin();
    std::string::const_iterator str_end = value.end();
    bool r = qi::phrase_parse(str_beg,str_end,qi::double_,ascii::space,*result);
    return r && (str_beg == str_end);
}

static bool string2int(const char * value, double * result)
{
    size_t length = strlen(value);
    if (length < 1 || value == NULL)
        return false;
    const char *begin = value;
    const char *iter  = begin;
    const char *end   = value + length;
    bool r = qi::phrase_parse(iter,end,qi::double_,ascii::space,*result);
    return r && (iter == end);
}


}
}

#endif // MAPNIK_CONVERSIONS_UTIL_HPP
