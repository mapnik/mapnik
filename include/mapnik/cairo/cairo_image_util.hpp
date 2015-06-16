/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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


#ifndef MAPNIK_CAIRO_IMAGE_UTIL_HPP
#define MAPNIK_CAIRO_IMAGE_UTIL_HPP

// mapnik
#include <mapnik/image.hpp>
#include <mapnik/cairo/cairo_context.hpp> // for cairo_surface_ptr

// stl
#include <stdexcept>

namespace mapnik {

static inline void cairo_image_to_rgba8(mapnik::image_rgba8 & data,
                                        cairo_surface_ptr const& surface)
{
    if (cairo_image_surface_get_format(&*surface) != CAIRO_FORMAT_ARGB32)
    {
        throw std::runtime_error("Unable to convert this Cairo format to rgba8 image");
    }

    if (cairo_image_surface_get_width(&*surface) != static_cast<int>(data.width()) ||
        cairo_image_surface_get_height(&*surface) != static_cast<int>(data.height()))
    {
        throw std::runtime_error("Mismatch in dimensions: size of image must match side of cairo surface");
    }

    int stride = cairo_image_surface_get_stride(&*surface) / 4;

    const std::unique_ptr<unsigned int[]> out_row(new unsigned int[data.width()]);
    const unsigned int *in_row = (const unsigned int *)cairo_image_surface_get_data(&*surface);

    for (unsigned int row = 0; row < data.height(); row++, in_row += stride)
    {
        for (unsigned int column = 0; column < data.width(); column++)
        {
            unsigned int in = in_row[column];
            unsigned int a = (in >> 24) & 0xff;
            unsigned int r = (in >> 16) & 0xff;
            unsigned int g = (in >> 8) & 0xff;
            unsigned int b = (in >> 0) & 0xff;

#define DE_ALPHA(x) do {                        \
                if (a == 0) x = 0;              \
                else x = x * 255 / a;           \
                if (x > 255) x = 255;           \
            } while(0)

            DE_ALPHA(r);
            DE_ALPHA(g);
            DE_ALPHA(b);

            out_row[column] = color(r, g, b, a).rgba();
        }
        data.set_row(row, out_row.get(), data.width());
    }
}

}


#endif // MAPNIK_CAIRO_IMAGE_UTIL_HPP
