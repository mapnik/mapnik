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

#ifndef MAPNIK_CAIRO_IO_HPP
#define MAPNIK_CAIRO_IO_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/map.hpp>

// stl
#include <string>

namespace mapnik {

#if defined(HAVE_CAIRO)
MAPNIK_DECL void save_to_cairo_file(mapnik::Map const& map,
                                    std::string const& filename,
                                    double scale_factor = 1.0,
                                    double scale_denominator = 0.0);
MAPNIK_DECL void save_to_cairo_file(mapnik::Map const& map,
                                    std::string const& filename,
                                    std::string const& type,
                                    double scale_factor = 1.0,
                                    double scale_denominator = 0.0);
#endif

} // namespace mapnik

#endif // MAPNIK_CAIRO_IO_HPP
