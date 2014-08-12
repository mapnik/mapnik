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

// mapnik
#include <mapnik/config.hpp>

// icu
#include <unicode/uversion.h> // for U_NAMESPACE_QUALIFIER

// stl
#include <iosfwd>
#include <cstddef>

namespace U_ICU_NAMESPACE {
    class UnicodeString;
}

namespace mapnik  {

#ifdef BIGINT
//using value_integer = boost::long_long_type;
using value_integer = long long;
#else
using value_integer = int;
#endif

using value_double = double;
using value_unicode_string = U_NAMESPACE_QUALIFIER UnicodeString;
using value_bool = bool;

struct MAPNIK_DECL value_null
{
    bool operator==(value_null const&) const
    {
        return true;
    }

    template <typename T>
    bool operator==(T const&) const
    {
        return false;
    }

    bool operator!=(value_null const&) const
    {
        return false;
    }

    template <typename T>
    bool operator!=(T const&) const
    {
        return true;
    }

    template <typename T>
    value_null operator+ (T const&) const
    {
        return *this;
    }

    template <typename T>
    value_null operator- (T const&) const
    {
        return *this;
    }

    template <typename T>
    value_null operator* (T const&) const
    {
        return *this;
    }

    template <typename T>
    value_null operator/ (T const&) const
    {
        return *this;
    }

    template <typename T>
    value_null operator% (T const&) const
    {
        return *this;
    }
};

inline std::size_t hash_value(value_null const&)
{
    return 0;
}

template <typename TChar, typename TTraits>
inline std::basic_ostream<TChar, TTraits>& operator<<(std::basic_ostream<TChar, TTraits>& out, value_null const& v) {
    return out;
}

inline std::istream& operator>> ( std::istream & s, value_null & )
{
    return s;
}


} // namespace mapnik

#endif // MAPNIK_VALUE_TYPES_HPP
