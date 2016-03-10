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

#ifndef MAPNIK_VALUE_TYPES_HPP
#define MAPNIK_VALUE_TYPES_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/pixel_types.hpp>


#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <unicode/uversion.h> // for U_NAMESPACE_QUALIFIER
#pragma GCC diagnostic pop

// stl
#include <type_traits>
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
// to mapnik::value_type conversions traits
template <typename T>
struct is_value_bool
{
    constexpr static bool value = std::is_same<T, bool>::value;
};

template <typename T>
struct is_value_integer
{
    constexpr static bool value = std::is_integral<T>::value && !std::is_same<T, bool>::value;
};

template <typename T>
struct is_value_double
{
    constexpr static bool value = std::is_floating_point<T>::value;
};

template <typename T>
struct is_value_unicode_string
{
    constexpr static bool value = std::is_same<T, typename mapnik::value_unicode_string>::value;
};

template <typename T>
struct is_value_string
{
    constexpr static bool value = std::is_same<T, typename std::string>::value;
};

template <typename T>
struct is_value_null
{
    constexpr static bool value = std::is_same<T, typename mapnik::value_null>::value;
};

template <typename T, class Enable = void>
struct mapnik_value_type
{
    using type = T;
};

// value_null
template <typename T>
struct mapnik_value_type<T, typename std::enable_if<detail::is_value_null<T>::value>::type>
{
    using type = mapnik::value_null;
};

// value_bool
template <typename T>
struct mapnik_value_type<T, typename std::enable_if<detail::is_value_bool<T>::value>::type>
{
    using type = mapnik::value_bool;
};

// value_integer
template <typename T>
struct mapnik_value_type<T, typename std::enable_if<detail::is_value_integer<T>::value>::type>
{
    using type = mapnik::value_integer;
};

// value_double
template <typename T>
struct mapnik_value_type<T, typename std::enable_if<detail::is_value_double<T>::value>::type>
{
    using type = mapnik::value_double;
};

// value_unicode_string
template <typename T>
struct mapnik_value_type<T, typename std::enable_if<detail::is_value_unicode_string<T>::value>::type>
{
    using type = mapnik::value_unicode_string const&;
};

template <typename T>
using mapnik_value_type_decay = mapnik_value_type<typename std::decay<T>::type>;

template <typename T, typename U>
using is_same_decay = std::is_same<typename std::decay<T>::type,
                                   typename std::decay<U>::type>;

} // namespace detail

} // namespace mapnik

#endif // MAPNIK_VALUE_TYPES_HPP
