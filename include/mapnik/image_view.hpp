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

#ifndef MAPNIK_IMAGE_VIEW_HPP
#define MAPNIK_IMAGE_VIEW_HPP

#include <mapnik/image.hpp>

namespace mapnik {

template <typename T>
class image_view
{
public:
    using pixel = typename T::pixel;
    using pixel_type = typename T::pixel_type;
    static const image_dtype dtype = T::dtype;
    static constexpr std::size_t pixel_size = sizeof(pixel_type);
    
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
    inline const pixel_type& operator() (std::size_t i, std::size_t j) const
    {
        return data_(i,j);
    }

    inline unsigned getSize() const
    {
        return height_ * width_ * pixel_size;
    }

    inline unsigned getRowSize() const
    {
        return width_ * pixel_size;
    }

    inline const pixel_type* getRow(unsigned row) const
    {
        return data_.getRow(row + y_) + x_;
    }
    
    inline const pixel_type* getRow(unsigned row, std::size_t x0) const
    {
        return data_.getRow(row + y_, x0) + x_;
    }

    inline T const& data() const
    {
        return data_;
    }

    inline bool get_premultiplied() const
    {
        return data_.get_premultiplied();
    }

    inline double get_offset() const
    {
        return data_.get_offset();
    }

    inline double get_scaling() const
    {
        return data_.get_scaling();
    }
    
    inline image_dtype get_dtype() const
    {
        return dtype;
    }

private:
    unsigned x_;
    unsigned y_;
    unsigned width_;
    unsigned height_;
    T const& data_;
};

using image_view_rgba8 = image_view<image_rgba8>;
using image_view_gray8 = image_view<image_gray8>;
using image_view_gray8s = image_view<image_gray8s>;
using image_view_gray16 = image_view<image_gray16>;
using image_view_gray16s = image_view<image_gray16s>;
using image_view_gray32 = image_view<image_gray32>;
using image_view_gray32s = image_view<image_gray32s>;
using image_view_gray32f = image_view<image_gray32f>;
using image_view_gray64 = image_view<image_gray64>;
using image_view_gray64s = image_view<image_gray64s>;
using image_view_gray64f = image_view<image_gray64f>;

} // end ns

#endif // MAPNIK_IMAGE_VIEW_HPP
