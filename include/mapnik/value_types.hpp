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

#ifndef MAPNIK_VALUE_TYPES_HPP
#define MAPNIK_VALUE_TYPES_HPP

// icu
#include <unicode/unistr.h>  // for UnicodeString

// boost
//#include <boost/cstdint.hpp>

// stl
#include <iosfwd> // for ostream

namespace mapnik  {

#ifdef BIGINT
//typedef boost::long_long_type value_integer;
typedef long long value_integer;
#else
typedef int value_integer;
#endif

typedef double value_double;
typedef UnicodeString  value_unicode_string;
typedef bool value_bool;

struct value_null
{
    template <typename T>
    value_null operator+ (T const& /*other*/) const
    {
        return *this;
    }

    template <typename T>
    value_null operator- (T const& /*other*/) const
    {
        return *this;
    }

    template <typename T>
    value_null operator* (T const& /*other*/) const
    {
        return *this;
    }

    template <typename T>
    value_null operator/ (T const& /*other*/) const
    {
        return *this;
    }

    template <typename T>
    value_null operator% (T const& /*other*/) const
    {
        return *this;
    }
};

inline std::size_t hash_value(const value_null& val) {
    return 0;
}

inline std::ostream& operator<< (std::ostream & out,value_null const& v)
{
    return out;
}

} // namespace mapnik

#endif // MAPNIK_VALUE_TYPES_HPP
