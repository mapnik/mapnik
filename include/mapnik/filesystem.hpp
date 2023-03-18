/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2023 Artem Pavlenko
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

#ifndef MAPNIK_FILESYSTEM_HPP
#define MAPNIK_FILESYSTEM_HPP

#if (__cplusplus >= 201703L) && !defined(USE_BOOST_FILESYSTEM)
#include <filesystem>
#else
#include <boost/filesystem/operations.hpp> // for absolute, exists, etc
#include <boost/filesystem/path.hpp>       // for path, operator/
#endif

namespace mapnik {
#if defined(__cpp_lib_filesystem) && !defined(USE_BOOST_FILESYSTEM)
namespace fs = std::filesystem;
using error_code = std::error_code;
#else
namespace fs = boost::filesystem;
using error_code = boost::system::error_code;
#endif
} // namespace mapnik

#endif // MAPNIK_FILESYSTEM_HPP
