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

}
}

#endif // MAPNIK_CONVERSIONS_UTIL_HPP
