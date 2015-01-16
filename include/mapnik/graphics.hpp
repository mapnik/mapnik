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
    boost::optional<color> background_;
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

    inline void clear()
    {
        std::fill(data_.getData(), data_.getData() + data_.width() * data_.height(), 0);
    }

    boost::optional<color> const& get_background() const;

    void set_background(const color& c);

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

    inline void set_rectangle(int x0,int y0,image_data_rgba8 const& data)
    {
        box2d<int> ext0(0,0,data_.width(),data_.height());
        box2d<int> ext1(x0,y0,x0+data.width(),y0+data.height());

        if (ext0.intersects(ext1))
        {
            box2d<int> box = ext0.intersect(ext1);
            for (int y = box.miny(); y < box.maxy(); ++y)
            {
                unsigned int* row_to =  data_.getRow(y);
                unsigned int const * row_from = data.getRow(y-y0);

                for (int x = box.minx(); x < box.maxx(); ++x)
                {
                    if (row_from[x-x0] & 0xff000000)
                    {
                        row_to[x] = row_from[x-x0];
                    }
                }
            }
        }
    }

    template <typename MergeMethod>
        inline void merge_rectangle(image_data_rgba8 const& data, unsigned x0, unsigned y0, float opacity)
    {
        box2d<int> ext0(0,0,data_.width(),data_.height());
        box2d<int> ext1(x0,y0,x0 + data.width(),y0 + data.height());

        if (ext0.intersects(ext1))
        {
            box2d<int> box = ext0.intersect(ext1);
            for (int y = box.miny(); y < box.maxy(); ++y)
            {
                unsigned int* row_to =  data_.getRow(y);
                unsigned int const * row_from = data.getRow(y-y0);
                for (int x = box.minx(); x < box.maxx(); ++x)
                {
                    unsigned rgba0 = row_to[x];
                    unsigned rgba1 = row_from[x-x0];
                    unsigned a1 = int( ((rgba1 >> 24) & 0xff) * opacity );
                    if (a1 == 0) continue;
                    unsigned r1 = rgba1 & 0xff;
                    unsigned g1 = (rgba1 >> 8 ) & 0xff;
                    unsigned b1 = (rgba1 >> 16) & 0xff;

                    unsigned a0 = (rgba0 >> 24) & 0xff;
                    unsigned r0 = rgba0 & 0xff ;
                    unsigned g0 = (rgba0 >> 8 ) & 0xff;
                    unsigned b0 = (rgba0 >> 16) & 0xff;

                    unsigned a = (a1 * 255 + (255 - a1) * a0 + 127)/255;

                    MergeMethod::mergeRGB(r0,g0,b0,r1,g1,b1);

                    r0 = (r1*a1 + (((255 - a1) * a0 + 127)/255) * r0 + 127)/a;
                    g0 = (g1*a1 + (((255 - a1) * a0 + 127)/255) * g0 + 127)/a;
                    b0 = (b1*a1 + (((255 - a1) * a0 + 127)/255) * b0 + 127)/a;

                    row_to[x] = (a << 24)| (b0 << 16) |  (g0 << 8) | (r0) ;
                }
            }
        }
    }
};
}

#endif // MAPNIK_GRAPHICS_HPP
