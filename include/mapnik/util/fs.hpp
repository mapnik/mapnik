/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_FS_HPP
#define MAPNIK_FS_HPP

// mapnik
#include <mapnik/config.hpp>

// stl
#include <string>
#include <vector>

namespace mapnik { namespace util {

MAPNIK_DECL bool exists(std::string const& value);
MAPNIK_DECL bool is_directory(std::string const& value);
MAPNIK_DECL bool is_regular_file(std::string const& value);
MAPNIK_DECL bool remove(std::string const& value);
MAPNIK_DECL bool is_relative(std::string const& value);
MAPNIK_DECL std::string make_relative(std::string const& filepath, std::string const& base);
MAPNIK_DECL std::string make_absolute(std::string const& filepath, std::string const& base);
MAPNIK_DECL std::string dirname(std::string const& value);
MAPNIK_DECL std::string basename(std::string const& value);
MAPNIK_DECL std::vector<std::string> list_directory(std::string const& value);

}}



#endif
