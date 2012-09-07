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

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/global.hpp>
#include <mapnik/color.hpp>

// agg
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"

// cairo
#ifdef HAVE_CAIRO
#include <cairomm/surface.h>
#endif

// boost
#include <boost/scoped_array.hpp>

// stl
#include <iostream>

namespace mapnik
{
image_32::image_32(int width,int height)
    :width_(width),
     height_(height),
     data_(width,height),
     painted_(false) {}

image_32::image_32(const image_32& rhs)
    :width_(rhs.width_),
     height_(rhs.height_),
     data_(rhs.data_),
     painted_(rhs.painted_)  {}

#ifdef HAVE_CAIRO
image_32::image_32(Cairo::RefPtr<Cairo::ImageSurface> rhs)
    :width_(rhs->get_width()),
     height_(rhs->get_height()),
     data_(rhs->get_width(),rhs->get_height())
{
    painted_ = true;
    if (rhs->get_format() != Cairo::FORMAT_ARGB32)
    {
        MAPNIK_LOG_WARN(graphics) << "Unable to convert this Cairo format";
        return; // throw exception ??
    }

    int stride = rhs->get_stride() / 4;

    boost::scoped_array<unsigned int> out_row(new unsigned int[width_]);
    const unsigned int *in_row = (const unsigned int *)rhs->get_data();

    for (unsigned int row = 0; row < height_; row++, in_row += stride)
    {
        for (unsigned int column = 0; column < width_; column++)
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
        data_.setRow(row, out_row.get(), width_);
    }
}
#endif

image_32::~image_32() {}

void image_32::set_grayscale_to_alpha()
{
    for (unsigned int y = 0; y < height_; ++y)
    {
        unsigned int* row_from = data_.getRow(y);
        for (unsigned int x = 0; x < width_; ++x)
        {
            unsigned rgba = row_from[x];
            // TODO - big endian support
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
    for (unsigned y = 0; y < height_; ++y)
    {
        unsigned int* row_from = data_.getRow(y);
        for (unsigned x = 0; x < width_; ++x)
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
    {
        for (unsigned int y = 0; y < height_; ++y)
        {
            unsigned int* row_to =  data_.getRow(y);
            for (unsigned int x = 0; x < width_; ++x)
            {
                unsigned rgba = row_to[x];

#ifdef MAPNIK_BIG_ENDIAN
                unsigned a0 = (rgba & 0xff);
                unsigned a1 = int( (rgba & 0xff) * opacity );

                if (a0 == a1) continue;

                unsigned r = (rgba >> 24) & 0xff;
                unsigned g = (rgba >> 16 ) & 0xff;
                unsigned b = (rgba >> 8) & 0xff;

                row_to[x] = (a1) | (b << 8) |  (g << 16) | (r << 24) ;

#else
                unsigned a0 = (rgba >> 24) & 0xff;
                unsigned a1 = int( ((rgba >> 24) & 0xff) * opacity );
                //unsigned a1 = opacity;
                if (a0 == a1) continue;

                unsigned r = rgba & 0xff;
                unsigned g = (rgba >> 8 ) & 0xff;
                unsigned b = (rgba >> 16) & 0xff;

                row_to[x] = (a1 << 24)| (b << 16) |  (g << 8) | (r) ;
#endif
            }
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

void image_32::premultiply()
{
    agg::rendering_buffer buffer(data_.getBytes(),width_,height_,width_ * 4);
    agg::pixfmt_rgba32 pixf(buffer);
    pixf.premultiply();
}

void image_32::demultiply()
{
    agg::rendering_buffer buffer(data_.getBytes(),width_,height_,width_ * 4);
    agg::pixfmt_rgba32 pixf(buffer);
    pixf.demultiply();
}

void image_32::composite_pixel(unsigned op, int x,int y, unsigned c, unsigned cover, double opacity)
{
    typedef agg::rgba8 color_type;
    typedef color_type::value_type value_type;
    typedef agg::order_rgba order_type;
    typedef agg::comp_op_adaptor_rgba<color_type,order_type> blender_type;
         
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
