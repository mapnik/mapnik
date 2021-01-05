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

#ifndef MAPNIK_IMAGE_OPTIONS_HPP
#define MAPNIK_IMAGE_OPTIONS_HPP

#include <map>
#include <string>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/optional.hpp>
#pragma GCC diagnostic pop


namespace mapnik {

using image_options_map = std::map<std::string, boost::optional<std::string> >;
inline std::string to_string(boost::optional<std::string> const& val) { return val ? *val : "<unitialised>";}
image_options_map parse_image_options(std::string const& options);

}

#endif // MAPNIK_IMAGE_OPTIONS_HPP
