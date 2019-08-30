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

#ifndef MAPNIK_VERSION_HPP
#define MAPNIK_VERSION_HPP

#include <mapnik/stringify_macro.hpp>

#define MAPNIK_MAJOR_VERSION 4
#define MAPNIK_MINOR_VERSION 0
#define MAPNIK_PATCH_VERSION 0

#define MAPNIK_VERSION          MAPNIK_VERSION_ENCODE(MAPNIK_MAJOR_VERSION, \
                                                      MAPNIK_MINOR_VERSION, \
                                                      MAPNIK_PATCH_VERSION)

#define MAPNIK_VERSION_STRING   MAPNIK_STRINGIFY(MAPNIK_MAJOR_VERSION) "." \
                                MAPNIK_STRINGIFY(MAPNIK_MINOR_VERSION) "." \
                                MAPNIK_STRINGIFY(MAPNIK_PATCH_VERSION)

#define MAPNIK_VERSION_AT_LEAST(major, minor, patch) \
        (MAPNIK_VERSION >= MAPNIK_VERSION_ENCODE(major, minor, patch))

#define MAPNIK_VERSION_ENCODE(major, minor, patch) \
        ((major) * 100000 + (minor) * 100 + (patch))

#endif // MAPNIK_VERSION_HPP
