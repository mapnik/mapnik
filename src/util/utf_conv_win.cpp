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

#ifdef _WIN32
// windows specific methods for UTF8 from/to UTF16
#include <mapnik/util/utf_conv_win.hpp>
#include <string>
#include <vector>
#define NOMINMAX
#include <windows.h>

namespace mapnik {

std::string utf16_to_utf8(std::wstring const& wstr)
{
    std::string str;
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, 0, 0, 0, 0);
    if(size > 0)
    {
        std::vector<char> buffer(size);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &buffer[0], size, 0, 0);
        str.assign(buffer.begin(), buffer.end() - 1);
    }
    return str;
}

std::wstring utf8_to_utf16 (std::string const& str)
{
    std::wstring wstr;
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, 0, 0);
    if (size > 0)
    {
        std::vector<wchar_t> buffer(size);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &buffer[0], size);
        wstr.assign(buffer.begin(), buffer.end() - 1);
    }
    return wstr;
}

} // namespace mapnik

#endif // _WIN32
