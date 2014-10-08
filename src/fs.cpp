/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

//#define BOOST_FILESYSTEM_VERSION 2

// mapnik
#include <mapnik/utils.hpp>
#include <mapnik/util/fs.hpp>

// boost
#include <boost/filesystem/convenience.hpp>

// stl
#include <stdexcept>
#include <cstdlib>
#include <iostream>

#if (BOOST_FILESYSTEM_VERSION <= 2)


namespace boost {
namespace filesystem {
path read_symlink(const path& p)
{
    path symlink_path;

#ifdef BOOST_POSIX_API
    for (std::size_t path_max = 64;; path_max *= 2)// loop 'til buffer is large enough
    {
        const std::unique_ptr<char[]> buf(new char[path_max]);
        ssize_t result;
        if ((result=::readlink(p.string().c_str(), buf.get(), path_max))== -1)
        {
            throw std::runtime_error("could not read symlink");
        }
        else
        {
            if(result != static_cast<ssize_t>(path_max))
            {
                symlink_path.assign(buf.get(), buf.get() + result);
                break;
            }
        }
    }
#endif
    return symlink_path;
}
}
}
#endif

namespace mapnik {

namespace util {

    bool exists(std::string const& filepath)
    {
#ifdef _WINDOWS
        return boost::filesystem::exists(mapnik::utf8_to_utf16(filepath));
#else
        return boost::filesystem::exists(filepath);
#endif
    }

    bool is_directory(std::string const& filepath)
    {
#ifdef _WINDOWS
        return boost::filesystem::is_directory(mapnik::utf8_to_utf16(filepath));
#else
        return boost::filesystem::is_directory(filepath);
#endif
    }

    bool is_regular_file(std::string const& filepath)
    {
#ifdef _WINDOWS
        return boost::filesystem::is_regular_file(mapnik::utf8_to_utf16(filepath));
#else
        return boost::filesystem::is_regular_file(filepath);
#endif
    }

    bool remove(std::string const& filepath)
    {
#ifdef _WINDOWS
        return boost::filesystem::remove(mapnik::utf8_to_utf16(filepath));
#else
        return boost::filesystem::remove(filepath);
#endif
    }

    bool is_relative(std::string const& filepath)
    {

#ifdef _WINDOWS
        boost::filesystem::path child_path(mapnik::utf8_to_utf16(filepath));
#else
        boost::filesystem::path child_path(filepath);
#endif
        return (! child_path.has_root_directory() && ! child_path.has_root_name());
    }


    std::string make_relative(std::string const& filepath, std::string const& base)
    {
#ifdef _WINDOWS
        boost::filesystem::path absolute_path(mapnik::utf8_to_utf16(base));
#else
        boost::filesystem::path absolute_path(base);
#endif
        // support symlinks
        if (boost::filesystem::is_symlink(absolute_path))
        {
            absolute_path = boost::filesystem::read_symlink(absolute_path);
        }
#if (BOOST_FILESYSTEM_VERSION == 3)
        return boost::filesystem::absolute(absolute_path.parent_path() / filepath).string();
#else
        return boost::filesystem::complete(absolute_path.branch_path() / filepath).normalize().string();
#endif
    }

    std::string make_absolute(std::string const& filepath, std::string const& base)
    {
/*
#if (BOOST_FILESYSTEM_VERSION == 3)
        // TODO - normalize is now deprecated, use make_preferred?
        return boost::filesystem::absolute(boost::filesystem::path(base)/filepath).string();
#else // v2
        return boost::filesystem::complete(boost::filesystem::path(base)/filepath).normalize().string();
#endif
*/
        // http://insanecoding.blogspot.com/2007/11/directory-safety-when-working-with.html
        char * _base = realpath(base.c_str(),NULL);
        if (_base != nullptr) {
            std::string absolute(_base);
            free(_base);
            absolute += "/" + filepath;
            //std::clog << "abs " << absolute << "\n";
            return absolute;
            /*
            char * res = realpath(absolute.c_str(),NULL);
            if (res == nullptr) // get here for files that do not exist like ../[this].png
            {
                return absolute;
            }
            else
            {
                std::string result(res);
                std::clog << "res " << result << "\n";
                free(res);
                return result;
            }
            */
        }
        return filepath;
    }

    std::string dirname(std::string const& filepath)
    {
        boost::filesystem::path bp(filepath);
#if (BOOST_FILESYSTEM_VERSION == 3)
        return bp.parent_path().string();
#else // v2
        return bp.branch_path().string();
#endif
    }

} // end namespace util

} // end namespace mapnik
