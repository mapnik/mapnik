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

#ifndef MAPNIK_AGG_PATTERN_SOURCE_HPP
#define MAPNIK_AGG_PATTERN_SOURCE_HPP

// mapnik
#include <mapnik/image_data.hpp>
#include <mapnik/noncopyable.hpp>

// agg
#include "agg_color_rgba.h"

namespace mapnik
{

class pattern_source : private mapnik::noncopyable
{
public:
    pattern_source(image_data_32 const& pattern)
        : pattern_(pattern) {}

    unsigned int width() const
    {
        return pattern_.width();
    }
    unsigned int height() const
    {
        return pattern_.height();
    }
    agg::rgba8 pixel(int x, int y) const
    {
        unsigned c = pattern_(x,y);
        return agg::rgba8(c & 0xff,
                          (c >> 8) & 0xff,
                          (c >> 16) & 0xff,
                          (c >> 24) & 0xff);
    }
private:
    image_data_32 const& pattern_;
};
}

#endif // MAPNIK_AGG_PATTERN_SOURCE_HPP
