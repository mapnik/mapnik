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

#ifndef MAPNIK_SAFE_CAST_HPP
#define MAPNIK_SAFE_CAST_HPP

#include <limits>
#include <type_traits>
#include <cstdint>

namespace mapnik {

namespace detail {

template<typename T, typename S, typename E = void>
struct numeric_compare;

template<typename T, typename S>
struct numeric_compare_same_sign
{
    using sizeup = typename std::conditional<sizeof(T) >= sizeof(S), T, S>::type;

    static inline bool less(T t, S s) { return static_cast<sizeup>(t) < static_cast<sizeup>(s); }

    static inline bool greater(T t, S s) { return static_cast<sizeup>(t) > static_cast<sizeup>(s); }
};

template<typename T, typename S>
struct numeric_compare<T,
                       S,
                       typename std::enable_if<!std::is_floating_point<T>::value && !std::is_floating_point<S>::value &&
                                               ((std::is_unsigned<T>::value && std::is_unsigned<S>::value) ||
                                                (std::is_signed<T>::value && std::is_signed<S>::value))>::type>
    : numeric_compare_same_sign<T, S>
{};

template<typename T, typename S>
struct numeric_compare<
  T,
  S,
  typename std::enable_if<!std::is_floating_point<T>::value && !std::is_floating_point<S>::value &&
                          std::is_integral<T>::value && std::is_signed<T>::value && std::is_unsigned<S>::value>::type>
{
    static inline bool less(T t, S s)
    {
        return (t < static_cast<T>(0)) ? true : static_cast<std::uint64_t>(t) < static_cast<std::uint64_t>(s);
    }

    static inline bool greater(T t, S s)
    {
        return (t < static_cast<T>(0)) ? false : static_cast<std::uint64_t>(t) > static_cast<std::uint64_t>(s);
    }
};

template<typename T, typename S>
struct numeric_compare<
  T,
  S,
  typename std::enable_if<!std::is_floating_point<T>::value && !std::is_floating_point<S>::value &&
                          std::is_integral<T>::value && std::is_unsigned<T>::value && std::is_signed<S>::value>::type>
{
    static inline bool less(T t, S s)
    {
        return (s < static_cast<S>(0)) ? false : static_cast<std::uint64_t>(t) < static_cast<std::uint64_t>(s);
    }

    static inline bool greater(T t, S s)
    {
        return (s < static_cast<S>(0)) ? true : static_cast<std::uint64_t>(t) > static_cast<std::uint64_t>(s);
    }
};

template<typename T, typename S>
struct numeric_compare<
  T,
  S,
  typename std::enable_if<std::is_floating_point<T>::value && std::is_floating_point<S>::value>::type>
{
    static inline bool less(T t, S s) { return t < s; }

    static inline bool greater(T t, S s) { return t > s; }
};

template<typename T, typename S>
struct numeric_compare<T,
                       S,
                       typename std::enable_if<(std::is_floating_point<T>::value && std::is_integral<S>::value) ||
                                               (std::is_integral<T>::value && std::is_floating_point<S>::value)>::type>
{
    static inline bool less(T t, S s)
    {
        return numeric_compare<double, double>::less(static_cast<double>(t), static_cast<double>(s));
    }

    static inline bool greater(T t, S s)
    {
        return numeric_compare<double, double>::greater(static_cast<double>(t), static_cast<double>(s));
    }
};

// floats
template<typename T, typename Enable = void>
struct bounds
{
    static const T lowest() { return static_cast<T>(-std::numeric_limits<T>::max()); }
    static const T highest() { return std::numeric_limits<T>::max(); }
};

// integers
template<typename T>
struct bounds<T, typename std::enable_if<std::numeric_limits<T>::is_integer>::type>
{
    static const T lowest() { return std::numeric_limits<T>::min(); }
    static const T highest() { return std::numeric_limits<T>::max(); }
};

} // namespace detail

template<typename T, typename S>
inline T safe_cast(S s)
{
    static const auto max_val = detail::bounds<T>::highest();
    static const auto min_val = detail::bounds<T>::lowest();

    if (detail::numeric_compare<S, T>::greater(s, max_val))
    {
        return max_val;
    }
    else if (detail::numeric_compare<S, T>::less(s, min_val))
    {
        return min_val;
    }
    else
    {
        return static_cast<T>(s);
    }
}

} // namespace mapnik

#endif // MAPNIK_SAFE_CAST_HPP
