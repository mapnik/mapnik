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

#ifndef MAPNIK_UTIL_UTF_CONV_WIN_HPP
#define MAPNIK_UTIL_UTF_CONV_WIN_HPP

#ifdef _WIN32
// mapnik
#include <mapnik/config.hpp>
// stl
#include <string>

namespace mapnik {

// UTF8 <--> UTF16 conversion routines

MAPNIK_DECL std::string utf16_to_utf8(std::wstring const& wstr);
MAPNIK_DECL std::wstring utf8_to_utf16(std::string const& str);

} // namespace mapnik
#endif // _WIN32

#endif // MAPNIK_UTIL_UTF_CONV_WIN_HPP
