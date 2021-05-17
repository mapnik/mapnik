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

#ifndef FROM_U8STRING_HPP
#define FROM_U8STRING_HPP

// stl
#include <string>
#include <utility>

// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1423r2.html
// Explicit conversion functions can be used, in a C++17 compatible manner, to cope with the change of return type to// the std::filesystem::path member functions when a UTF-8 encoded path is desired in an object of type std::string.

namespace mapnik { namespace util {

inline std::string from_u8string(std::string const& s) {
  return s;
}

inline std::string from_u8string(std::string &&s) {
  return std::move(s);
}
#if defined(__cpp_lib_char8_t)
inline std::string from_u8string(std::u8string const&s) {
  return std::string(s.begin(), s.end());
}
#endif
/*
template<std::size_t N>
struct char_array {
    template<std::size_t P, std::size_t... I>
    constexpr char_array(
        const char (&r)[P],
        std::index_sequence<I...>)
        :
        data{(I<P?r[I]:'\0')...}
    {}
    template<std::size_t P, typename = std::enable_if_t<(P<=N)>>
    constexpr char_array(const char(&r)[P])
        : char_array(r, std::make_index_sequence<N>())
    {}

#if defined(__cpp_char8_t)
    template<std::size_t P, std::size_t... I>
    constexpr char_array(
        const char8_t (&r)[P],
        std::index_sequence<I...>)
        :
        data{(I<P?static_cast<char>(r[I]):'\0')...}
    {}
    template<std::size_t P, typename = std::enable_if_t<(P<=N)>>
    constexpr char_array(const char8_t(&r)[P])
        : char_array(r, std::make_index_sequence<N>())
    {}
#endif

    constexpr (&operator const char() const)[N] {
        return data;
    }
    constexpr (&operator char())[N] {
        return data;
    }

    char data[N];
};

template<std::size_t N>
char_array(const char(&)[N]) -> char_array<N>;

#if defined(__cpp_char8_t)
template<std::size_t N>
char_array(const char8_t(&)[N]) -> char_array<N>;
#endif
*/
}} // end of namespace mapnik

#endif // FROM_U8STRING_HPP
