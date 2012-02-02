/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef SHAPE_UTILS_HPP
#define SHAPE_UTILS_HPP

// mapnik
#include <mapnik/feature.hpp>
#include "shape_io.hpp"
// stl
#include <set>
#include <vector>
#include <string>

void setup_attributes(mapnik::context_ptr const& ctx,
                      std::set<std::string> const& names,
                      std::string const& shape_name,
                      shape_io & shape,
                      std::vector<int> & attr_ids);

#endif // SHAPE_UTILS_HPP
