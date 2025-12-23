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

#ifndef PLUGINS_INPUT_OGR_OGR_UTILS_HPP_
#define PLUGINS_INPUT_OGR_OGR_UTILS_HPP_

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace ogr_utils {

using option_ptr = std::unique_ptr<char[], std::function<void(char*)>>;

std::vector<option_ptr> split_open_options(std::string const& options);

char** open_options_for_ogr(std::vector<ogr_utils::option_ptr> const& options);

} // namespace ogr_utils

#endif /* PLUGINS_INPUT_OGR_OGR_UTILS_HPP_ */
