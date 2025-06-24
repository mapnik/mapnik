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

namespace mapnik {
namespace util {

constexpr unsigned name_to_int(const char* str, unsigned off = 0)
{
    return !str[off] ? 5381 : (name_to_int(str, off + 1) * 33) ^ static_cast<unsigned>(str[off]);
}

constexpr unsigned operator"" _case(char const* str, std::size_t)
{
    return name_to_int(str);
}

} // namespace util
} // namespace mapnik
