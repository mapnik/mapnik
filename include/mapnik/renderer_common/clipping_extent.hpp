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

#ifndef MAPNIK_CLIPPING_EXTENT_HPP
#define MAPNIK_CLIPPING_EXTENT_HPP

#include <mapnik/geometry/box2d.hpp>

namespace mapnik {

template<typename T>
box2d<double> clipping_extent(T const& common)
{
    if (common.t_.offset() > 0)
    {
        box2d<double> box = common.query_extent_;
        double scale = static_cast<double>(common.query_extent_.width()) / static_cast<double>(common.width_);
        // 3 is used here because at least 3 was needed for the 'style-level-compositing-tiled-0,1' visual test to pass
        // TODO - add more tests to hone in on a more robust #
        scale *= common.t_.offset() * 3;
        box.pad(scale);
        return box;
    }
    return common.query_extent_;
}

} // namespace mapnik

#endif // MAPNIK_CLIPPING_EXTENT_HPP
