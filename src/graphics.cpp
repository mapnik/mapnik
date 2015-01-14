/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/global.hpp>
#include <mapnik/color.hpp>

// agg
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"

#ifdef HAVE_CAIRO
#include <mapnik/cairo/cairo_context.hpp>
#endif

namespace mapnik
{
image_32::image_32(int width,int height)
   : data_(width,height),
     painted_(false) {}


image_32::image_32(image_32 const& rhs)
   : data_(rhs.data_),
     painted_(rhs.painted_) {}

image_32::~image_32() {}

void image_32::set_grayscale_to_alpha()
{
    for (unsigned int y = 0; y < data_.height(); ++y)
    {
        unsigned int* row_from = data_.getRow(y);
        for (unsigned int x = 0; x < data_.width(); ++x)
        {
            unsigned rgba = row_from[x];
            unsigned r = rgba & 0xff;
            unsigned g = (rgba >> 8 ) & 0xff;
            unsigned b = (rgba >> 16) & 0xff;

            // magic numbers for grayscale
            unsigned a = static_cast<unsigned>(std::ceil((r * .3) + (g * .59) + (b * .11)));

            row_from[x] = (a << 24)| (255 << 16) |  (255 << 8) | (255) ;
        }
    }
}

void image_32::set_color_to_alpha(const color& c)
{
    for (unsigned y = 0; y < data_.height(); ++y)
    {
        unsigned int* row_from = data_.getRow(y);
        for (unsigned x = 0; x < data_.width(); ++x)
        {
            unsigned rgba = row_from[x];
            unsigned r = rgba & 0xff;
            unsigned g = (rgba >> 8 ) & 0xff;
            unsigned b = (rgba >> 16) & 0xff;
            if (r == c.red() && g == c.green() && b == c.blue())
            {
                row_from[x] = 0;
            }
        }
    }
}

void image_32::set_alpha(float opacity)
{
    for (unsigned int y = 0; y < data_.height(); ++y)
    {
        unsigned int* row_to =  data_.getRow(y);
        for (unsigned int x = 0; x < data_.width(); ++x)
        {
            unsigned rgba = row_to[x];
            unsigned a0 = (rgba >> 24) & 0xff;
            unsigned a1 = int( ((rgba >> 24) & 0xff) * opacity );
            //unsigned a1 = opacity;
            if (a0 == a1) continue;

            unsigned r = rgba & 0xff;
            unsigned g = (rgba >> 8 ) & 0xff;
            unsigned b = (rgba >> 16) & 0xff;

            row_to[x] = (a1 << 24)| (b << 16) |  (g << 8) | (r) ;
        }
    }
}

void image_32::set_background(const color& c)
{
    background_=c;
    data_.set(background_->rgba());
}

boost::optional<color> const& image_32::get_background() const
{
    return background_;
}

void image_32::composite_pixel(unsigned op, int x,int y, unsigned c, unsigned cover, double opacity)
{
    using color_type = agg::rgba8;
    using value_type = color_type::value_type;
    using order_type = agg::order_rgba;
    using blender_type = agg::comp_op_adaptor_rgba<color_type,order_type>;

    if (checkBounds(x,y))
    {
        unsigned rgba = data_(x,y);
        unsigned ca = (unsigned)(((c >> 24) & 0xff) * opacity);
        unsigned cb = (c >> 16 ) & 0xff;
        unsigned cg = (c >> 8) & 0xff;
        unsigned cr = (c & 0xff);
        blender_type::blend_pix(op, (value_type*)&rgba, cr, cg, cb, ca, cover);
        data_(x,y) = rgba;
    }
}

}
