/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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
// $Id$

#ifndef MAP_LOADER_HPP
#define MAP_LOADER_HPP

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <mapnik/map.hpp>
#include <string>

namespace mapnik
{
    MAPNIK_DECL void load_map(Map & map, std::string const& filename, bool strict = false);
    MAPNIK_DECL void load_map_string(Map & map, std::string const& str, bool strict = false, std::string const& base_url="");
}

#endif // LOAD_MAP_HPP
