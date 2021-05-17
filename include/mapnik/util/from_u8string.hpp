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

// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1423r2.html
// Explicit conversion functions can be used, in a C++17 compatible manner, to cope with the change of return type to// the std::filesystem::path member functions when a UTF-8 encoded path is desired in an object of type std::string.

namespace mapnik { namespace util {

std::string from_u8string(const std::string &s) {
  return s;
}
std::string from_u8string(std::string &&s) {
  return std::move(s);
}
#if defined(__cpp_lib_char8_t)
std::string from_u8string(const std::u8string &s) {
  return std::string(s.begin(), s.end());
}
#endif

}} // end of namespace mapnik

#endif // FROM_U8STRING_HPP
