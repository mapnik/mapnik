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

#include <mapnik/image.hpp>
#include <mapnik/image_view.hpp>

namespace mapnik {

template <typename T>
image_view<T>::image_view(unsigned x, unsigned y, unsigned width, unsigned height, T const& data)
    : x_(x),
      y_(y),
      width_(width),
      height_(height),
      data_(data)
{
    if (x_ >= data_.width() && data_.width() > 0) x_ = data_.width() - 1;
    if (y_ >= data_.height() && data.height() > 0) y_ = data_.height() - 1;
    if (x_ + width_ > data_.width()) width_ = data_.width() - x_;
    if (y_ + height_ > data_.height()) height_ = data_.height() - y_;
}

template <typename T>
image_view<T>::~image_view() {}

template <typename T>
image_view<T>::image_view(image_view<T> const& rhs)
    : x_(rhs.x_),
      y_(rhs.y_),
      width_(rhs.width_),
      height_(rhs.height_),
      data_(rhs.data_) {}

template <typename T>
image_view<T> & image_view<T>::operator=(image_view<T> const& rhs)
{
    if (&rhs==this) return *this;
    x_ = rhs.x_;
    y_ = rhs.y_;
    width_ = rhs.width_;
    height_ = rhs.height_;
    data_ = rhs.data_;
    return *this;
}

template <typename T>
bool image_view<T>::operator==(image_view<T> const& rhs) const
{
    return rhs.data_.getBytes() == data_.getBytes();
}

template <typename T>
bool image_view<T>::operator<(image_view<T> const& rhs) const
{
    return data_.getSize() < rhs.data_.getSize();
}

template <typename T>
inline unsigned image_view<T>::x() const
{
    return x_;
}

template <typename T>
inline unsigned image_view<T>::y() const
{
    return y_;
}

template <typename T>
inline unsigned image_view<T>::width() const
{
    return width_;
}

template <typename T>
inline unsigned image_view<T>::height() const
{
    return height_;
}

template <typename T>
inline const typename image_view<T>::pixel_type& image_view<T>::operator() (std::size_t i, std::size_t j) const
{
    return data_(i,j);
}

template <typename T>
inline unsigned image_view<T>::getSize() const
{
    return height_ * width_ * pixel_size;
}

template <typename T>
inline unsigned image_view<T>::getRowSize() const
{
    return width_ * pixel_size;
}

template <typename T>
inline const typename image_view<T>::pixel_type* image_view<T>::getRow(unsigned row) const
{
    return data_.getRow(row + y_) + x_;
}

template <typename T>
inline const typename image_view<T>::pixel_type* image_view<T>::getRow(unsigned row, std::size_t x0) const
{
    return data_.getRow(row + y_, x0) + x_;
}

template <typename T>
inline T const& image_view<T>::data() const
{
    return data_;
}

template <typename T>
inline bool image_view<T>::get_premultiplied() const
{
    return data_.get_premultiplied();
}

template <typename T>
inline double image_view<T>::get_offset() const
{
    return data_.get_offset();
}

template <typename T>
inline double image_view<T>::get_scaling() const
{
    return data_.get_scaling();
}

template <typename T>
inline image_dtype image_view<T>::get_dtype() const
{
    return dtype;
}

} // end ns
