// SPDX-License-Identifier: LGPL-2.1-or-later
/*****************************************************************************
 *
 * This file is part of Mapnik Vector Tile Plugin
 *
 * Copyright (C) 2023 Geofabrik GmbH
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

#ifndef PLUGINS_INPUT_MBTILES_VECTOR_MVT_MESSAGE_HPP_
#define PLUGINS_INPUT_MBTILES_VECTOR_MVT_MESSAGE_HPP_

namespace mvt_message {
enum class tile : protozero::pbf_tag_type { layer = 3 };
enum class layer : protozero::pbf_tag_type { version = 15, name = 1, features = 2, keys = 3, values = 4, extent = 5 };
enum class value : protozero::pbf_tag_type {
    string_value = 1,
    float_value = 2,
    double_value = 3,
    int_value = 4,
    uint_value = 5,
    sint_value = 6,
    bool_value = 7
};
enum class feature : protozero::pbf_tag_type { id = 1, tags = 2, type = 3, geometry = 4 };
enum class geom_type : int32_t { unknown = 0, point = 1, linestring = 2, polygon = 3 };
}; // namespace mvt_message

#endif /* PLUGINS_INPUT_MBTILES_VECTOR_MVT_MESSAGE_HPP_ */
