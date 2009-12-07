/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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
//$Id: graphics.cpp 17 2005-03-08 23:58:43Z pavlenko $

// mapnik
#include <mapnik/graphics.hpp>
#include <mapnik/image_util.hpp>

// cairo
#ifdef HAVE_CAIRO
#include <cairomm/surface.h>
#endif

#include <iostream>

namespace mapnik
{
    Image32::Image32(int width,int height)
        :width_(width),
         height_(height),
         data_(width,height) {}

    Image32::Image32(const Image32& rhs)
        :width_(rhs.width_),
         height_(rhs.height_),
         data_(rhs.data_)  {}

#ifdef HAVE_CAIRO
    Image32::Image32(Cairo::RefPtr<Cairo::ImageSurface> rhs)
        :width_(rhs->get_width()),
         height_(rhs->get_height()),
         data_(rhs->get_width(),rhs->get_height())
        {
            if (rhs->get_format() != Cairo::FORMAT_ARGB32)
            {
                    std::cerr << "Unable to convert this Cairo format\n";
                    return; // throw exception ??
            }

            int stride = rhs->get_stride() / 4;

            unsigned int out_row[width_];
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

#define DE_ALPHA(x) do {                      \
                       if (a == 0) x = 0;     \
                       else x = x * 255 / a;  \
                       if (x > 255) x = 255;  \
                   } while(0)

                   DE_ALPHA(r);
                   DE_ALPHA(g);
                   DE_ALPHA(b);

                   out_row[column] = color(r, g, b, a).rgba();
                }
                data_.setRow(row, out_row, width_);
            }
        }
#endif

    Image32::~Image32() {}

    const ImageData32& Image32::data() const
    {
        return data_;
    }

    void Image32::setBackground(const color& background)
    {
        background_=background;
        data_.set(background_.rgba());
    }

    const color& Image32::getBackground() const
    {
        return background_;
    }
}
