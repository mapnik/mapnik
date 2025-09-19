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

#include "pmtiles_source.hpp"
#include "mbtiles_source.hpp"
#include <string>

namespace mapnik {

std::unique_ptr<tiles_source> tiles_source::get_source(std::string const& filename)
{
    if (filename.ends_with(".pmtiles"))
    {
        return std::make_unique<mapnik::pmtiles_source>(filename);
    }
    else if (filename.ends_with(".mbtiles"))
    {
        return std::make_unique<mapnik::mbtiles_source>(filename);
    }
    return nullptr;
}

} // namespace mapnik
