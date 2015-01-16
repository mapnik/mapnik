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

#ifndef MAPNIK_GRAPHICS_HPP
#define MAPNIK_GRAPHICS_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/color.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/image_view.hpp>
#include <mapnik/global.hpp>

// stl
#include <algorithm>
#include <string>
#include <memory>

// boost
#include <boost/optional/optional.hpp>

namespace mapnik
{

class MAPNIK_DECL image_32
{
private:
    image_data_rgba8 data_;
    bool painted_;
public:
    using pixel_type = typename image_data_rgba8::pixel_type;
    image_32(int width,int height);
    image_32(image_32 const& rhs);
    image_32(image_data_rgba8 && data);
    ~image_32();

    void painted(bool painted)
    {
        painted_ = painted;
    }

    bool painted() const
    {
        return painted_;
    }

    inline const image_data_rgba8& data() const
    {
        return data_;
    }

    inline image_data_rgba8& data()
    {
        return data_;
    }

    inline const unsigned char* raw_data() const
    {
        return data_.getBytes();
    }

    inline unsigned char* raw_data()
    {
        return data_.getBytes();
    }

    inline image_view_rgba8 get_view(unsigned x,unsigned y, unsigned w,unsigned h)
    {
        return image_view_rgba8(x,y,w,h,data_);
    }

private:

    inline bool checkBounds(int x, int y) const
    {
        return (x >= 0 && x < static_cast<int>(data_.width()) && y >= 0 && y < static_cast<int>(data_.height()));
    }

public:
    inline void setPixel(int x,int y,unsigned int rgba)
    {
        if (checkBounds(x,y))
        {
            data_(x,y)=rgba;
        }
    }

    void composite_pixel(unsigned op, int x,int y,unsigned c, unsigned cover, double opacity);

    inline unsigned width() const
    {
        return data_.width();
    }

    inline unsigned height() const
    {
        return data_.height();
    }

};
}

#endif // MAPNIK_GRAPHICS_HPP
