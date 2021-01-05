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

#ifndef MAPNIK_LIBXML2_LOADER_HPP
#define MAPNIK_LIBXML2_LOADER_HPP

// mapnik
#include <mapnik/config.hpp> // for MAPNIK_DECL

// stl
#include <string>

namespace mapnik
{
class MAPNIK_DECL xml_node;
MAPNIK_DECL void read_xml(std::string const & filename, xml_node &node);
MAPNIK_DECL void read_xml_string(std::string const & str, xml_node &node, std::string const & base_path="");
}

#endif // MAPNIK_LIBXML2_LOADER_HPP
