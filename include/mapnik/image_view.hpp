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

#ifndef MAPNIK_IMAGE_VIEW_HPP
#define MAPNIK_IMAGE_VIEW_HPP

namespace mapnik {

template <typename T>
class image_view
{
public:
    using pixel_type = typename T::pixel_type;

    image_view(unsigned x, unsigned y, unsigned width, unsigned height, T const& data)
        : x_(x),
          y_(y),
          width_(width),
          height_(height),
          data_(data)
    {
        if (x_ >= data_.width()) x_=data_.width()-1;
        if (y_ >= data_.height()) y_=data_.height()-1;
        if (x_ + width_ > data_.width()) width_= data_.width() - x_;
        if (y_ + height_ > data_.height()) height_= data_.height() - y_;
    }

    ~image_view() {}

    image_view(image_view<T> const& rhs)
        : x_(rhs.x_),
          y_(rhs.y_),
          width_(rhs.width_),
          height_(rhs.height_),
          data_(rhs.data_) {}

    image_view<T> & operator=(image_view<T> const& rhs)
    {
        if (&rhs==this) return *this;
        x_ = rhs.x_;
        y_ = rhs.y_;
        width_ = rhs.width_;
        height_ = rhs.height_;
        data_ = rhs.data_;
        return *this;
    }

    inline unsigned x() const
    {
        return x_;
    }

    inline unsigned y() const
    {
        return y_;
    }

    inline unsigned width() const
    {
        return width_;
    }

    inline unsigned height() const
    {
        return height_;
    }

    inline const pixel_type* getRow(unsigned row) const
    {
        return data_.getRow(row + y_) + x_;
    }

    inline const unsigned char* getBytes() const
    {
        return data_.getBytes();
    }
    inline T& data()
    {
        return data_;
    }
    inline T const& data() const
    {
        return data_;
    }

private:
    unsigned x_;
    unsigned y_;
    unsigned width_;
    unsigned height_;
    T const& data_;
};
}

#endif // MAPNIK_IMAGE_VIEW_HPP
