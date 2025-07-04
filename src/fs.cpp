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

// mapnik
#include <mapnik/util/utf_conv_win.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/filesystem.hpp>

// stl
#include <stdexcept>

namespace mapnik {

namespace util {

bool exists(std::string const& filepath)
{
#ifdef _WIN32
    return fs::exists(mapnik::utf8_to_utf16(filepath));
#else
    return fs::exists(filepath);
#endif
}

bool is_directory(std::string const& filepath)
{
#ifdef _WIN32
    return fs::is_directory(mapnik::utf8_to_utf16(filepath));
#else
    return fs::is_directory(filepath);
#endif
}

bool is_regular_file(std::string const& filepath)
{
#ifdef _WIN32
    return fs::is_regular_file(mapnik::utf8_to_utf16(filepath));
#else
    return fs::is_regular_file(filepath);
#endif
}

bool remove(std::string const& filepath)
{
#ifdef _WIN32
    return fs::remove(mapnik::utf8_to_utf16(filepath));
#else
    return fs::remove(filepath);
#endif
}

bool is_relative(std::string const& filepath)
{
#ifdef _WIN32
    fs::path child_path(mapnik::utf8_to_utf16(filepath));
#else
    fs::path child_path(filepath);
#endif
    return (!child_path.has_root_directory() && !child_path.has_root_name());
}

std::string make_relative(std::string const& filepath, std::string const& base)
{
#ifdef _WIN32
    fs::path absolute_path(mapnik::utf8_to_utf16(base));
#else
    fs::path absolute_path(base);
#endif
    // support symlinks
    if (fs::is_symlink(absolute_path))
    {
        absolute_path = fs::read_symlink(absolute_path);
    }
    return fs::absolute(absolute_path.parent_path() / filepath).string();
}

std::string make_absolute(std::string const& filepath, std::string const& base)
{
    // TODO - normalize is now deprecated, use make_preferred?
    return fs::absolute(fs::path(base) / filepath).string();
}

std::string dirname(std::string const& filepath)
{
    fs::path bp(filepath);
    return bp.parent_path().string();
}

std::string basename(std::string const& value)
{
    fs::path bp(value);
    return bp.filename().string();
}

std::vector<std::string> list_directory(std::string const& dir)
{
    std::vector<std::string> listing;
    fs::directory_iterator end_itr;
#ifdef _WIN32
    std::wstring wide_dir(mapnik::utf8_to_utf16(dir));
    for (fs::directory_iterator itr(wide_dir); itr != end_itr; ++itr)
    {
        listing.emplace_back(mapnik::utf16_to_utf8(itr->path().wstring()));
    }
#else
    for (fs::directory_iterator itr(dir); itr != end_itr; ++itr)
    {
        listing.emplace_back(itr->path().string());
    }
#endif
    return listing;
}

} // end namespace util

} // end namespace mapnik
