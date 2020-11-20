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

#ifndef MAPNIK_VALUE_TYPES_HPP
#define MAPNIK_VALUE_TYPES_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/cxx11_support.hpp>
#include <mapnik/pixel_types.hpp>


#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <unicode/uversion.h> // for U_NAMESPACE_QUALIFIER
MAPNIK_DISABLE_WARNING_POP

// stl
#include <iosfwd>
#include <cstddef>

namespace U_ICU_NAMESPACE {
    class UnicodeString;
}

namespace mapnik  {

#ifdef BIGINT
//using value_integer = boost::long_long_type;
//using value_integer = long long;
using value_integer = std::int64_t;
using value_integer_pixel = gray64s_t;
#else
//using value_integer = int;
using value_integer = std::int32_t;
using value_integer_pixel = gray32s_t;
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

    bool operator>(value_null) const
    {
        return false;
    }

    bool operator>=(value_null) const
    {
        return true;
    }

    bool operator<(value_null) const
    {
        return false;
    }

    bool operator<=(value_null) const
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
inline std::basic_ostream<TChar, TTraits>& operator<<(std::basic_ostream<TChar, TTraits>& out, value_null const&)
{
    return out;
}

inline std::istream& operator>> ( std::istream & s, value_null & )
{
    return s;
}


namespace detail {

// Helper metafunction for mapnik::value construction and assignment.
// Returns:
//  value_bool      if T is bool
//  value_integer   if T is an integral type (except bool)
//  value_double    if T is a floating-point type
//  T &&            otherwise

template <typename T, typename dT = decay_t<T>>
using mapnik_value_type_t =
    conditional_t<
        std::is_same<dT, bool>::value, value_bool,
        conditional_t<
            std::is_integral<dT>::value, value_integer,
            conditional_t<
                std::is_floating_point<dT>::value, value_double,
                T && >>>;

} // namespace detail

} // namespace mapnik

#endif // MAPNIK_VALUE_TYPES_HPP
