/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#ifndef MAPNIK_UTILS_PROCESS_GEOJSON_FILE_X3_HPP
#define MAPNIK_UTILS_PROCESS_GEOJSON_FILE_X3_HPP

#include <utility>
#include <string>

namespace mapnik {
namespace detail {

template<typename T>
std::pair<bool, typename T::value_type::first_type>
  process_geojson_file_x3(T& boxes, std::string const& filename, bool validate_features, bool verbose);

}
} // namespace mapnik

#endif // MAPNIK_UTILS_PROCESS_GEOJSON_FILE_X3_HPP
