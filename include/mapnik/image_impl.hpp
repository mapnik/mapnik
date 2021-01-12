/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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
#include <mapnik/image.hpp>
#include <mapnik/pixel_types.hpp>

// stl
#include <cassert>
#include <stdexcept>
#include <algorithm>

namespace mapnik {

namespace detail {

// IMAGE_DIMENSIONS
template <std::size_t max_size>
image_dimensions<max_size>::image_dimensions(int width, int height)
    : width_(width),
      height_(height)
{
    std::int64_t area = static_cast<std::int64_t>(width) * static_cast<std::int64_t>(height);
    if (width < 0) throw std::runtime_error("Invalid width for image dimensions requested");
    if (height < 0) throw std::runtime_error("Invalid height for image dimensions requested");
    if (area > static_cast<std::int64_t>(max_size))
        throw std::runtime_error("Image area too large based on image dimensions");
}

template <std::size_t max_size>
std::size_t image_dimensions<max_size>::width() const
{
    return width_;
}

template <std::size_t max_size>
std::size_t image_dimensions<max_size>::height() const
{
    return height_;
}

} // end detail ns

// IMAGE
template <typename T>
image<T>::image()
    : dimensions_(0,0),
      buffer_(0),
      offset_(0.0),
      scaling_(1.0),
      premultiplied_alpha_(false),
      painted_(false)
{}

template <typename T>
image<T>::image(int width, int height, unsigned char* data, bool premultiplied, bool painted)
    : dimensions_(width, height),
      buffer_(data, width * height * sizeof(pixel_size)),
      offset_(0.0),
      scaling_(1.0),
      premultiplied_alpha_(premultiplied),
      painted_(painted) {}

template <typename T>
image<T>::image(int width, int height, bool initialize, bool premultiplied, bool painted)
    : dimensions_(width, height),
      buffer_(dimensions_.width() * dimensions_.height() * pixel_size),
      offset_(0.0),
      scaling_(1.0),
      premultiplied_alpha_(premultiplied),
      painted_(painted)
{
    if (initialize)
    {
        std::fill(begin(), end(), 0);
    }
}

template <typename T>
image<T>::image(image<T> const& rhs)
    : dimensions_(rhs.dimensions_),
      buffer_(rhs.buffer_),
      offset_(rhs.offset_),
      scaling_(rhs.scaling_),
      premultiplied_alpha_(rhs.premultiplied_alpha_),
      painted_(rhs.painted_) {}

template <typename T>
image<T>::image(image<T> && rhs) noexcept
    : dimensions_(std::move(rhs.dimensions_)),
      buffer_(std::move(rhs.buffer_)),
      offset_(rhs.offset_),
      scaling_(rhs.scaling_),
      premultiplied_alpha_(rhs.premultiplied_alpha_),
      painted_(rhs.painted_)
{
    rhs.dimensions_ = { 0, 0 };
}

template <typename T>
image<T>& image<T>::operator=(image<T> rhs)
{
    swap(rhs);
    return *this;
}

template <typename T>
bool image<T>::operator==(image<T> const& rhs) const
{
    return rhs.bytes() == bytes();
}

template <typename T>
bool image<T>::operator<(image<T> const& rhs) const
{
    return size() < rhs.size();
}

template <typename T>
void image<T>::swap(image<T> & rhs)
{
    std::swap(dimensions_, rhs.dimensions_);
    std::swap(buffer_, rhs.buffer_);
    std::swap(offset_, rhs.offset_);
    std::swap(scaling_, rhs.scaling_);
    std::swap(premultiplied_alpha_, rhs.premultiplied_alpha_);
    std::swap(painted_, rhs.painted_);
}

template <typename T>
inline typename image<T>::pixel_type& image<T>::operator() (std::size_t i, std::size_t j)
{
    assert(i < dimensions_.width() && j < dimensions_.height());
    return *get_row(j, i);
}

template <typename T>
inline const typename image<T>::pixel_type& image<T>::operator() (std::size_t i, std::size_t j) const
{
    assert(i < dimensions_.width() && j < dimensions_.height());
    return *get_row(j, i);
}

template <typename T>
inline std::size_t image<T>::width() const
{
    return dimensions_.width();
}

template <typename T>
inline std::size_t image<T>::height() const
{
    return dimensions_.height();
}

template <typename T>
inline std::size_t image<T>::size() const
{
    return dimensions_.height() * dimensions_.width() * pixel_size;
}

template <typename T>
inline std::size_t image<T>::row_size() const
{
    return dimensions_.width() * pixel_size;
}

template <typename T>
inline void image<T>::set(pixel_type const& t)
{
    std::fill(begin(), end(), t);
}

template <typename T>
inline const typename image<T>::pixel_type* image<T>::data() const
{
    return reinterpret_cast<const pixel_type*>(buffer_.data());
}

template <typename T>
inline typename image<T>::pixel_type* image<T>::data()
{
    return reinterpret_cast<pixel_type*>(buffer_.data());
}

template <typename T>
inline const unsigned char* image<T>::bytes() const
{
    return buffer_.data();
}

template <typename T>
inline unsigned char* image<T>::bytes()
{
    return buffer_.data();
}

// iterator interface
template <typename T>
inline typename image<T>::iterator image<T>::begin() { return data(); }

template <typename T>
inline typename image<T>::iterator image<T>::end() { return data() + dimensions_.width() * dimensions_.height(); }

template <typename T>
inline typename image<T>::const_iterator image<T>::begin() const { return data(); }

template <typename T>
inline typename image<T>::const_iterator image<T>::end() const{ return data() + dimensions_.width() * dimensions_.height(); }


template <typename T>
inline typename image<T>::pixel_type const* image<T>::get_row(std::size_t row) const
{
    return data() + row * dimensions_.width();
}

template <typename T>
inline const typename image<T>::pixel_type* image<T>::get_row(std::size_t row, std::size_t x0) const
{
    return data() + row * dimensions_.width() + x0;
}

template <typename T>
inline typename image<T>::pixel_type* image<T>::get_row(std::size_t row)
{
    return data() + row * dimensions_.width();
}

template <typename T>
inline typename image<T>::pixel_type* image<T>::get_row(std::size_t row, std::size_t x0)
{
    return data() + row * dimensions_.width() + x0;
}

template <typename T>
inline void image<T>::set_row(std::size_t row, pixel_type const* buf, std::size_t size)
{
    assert(row < dimensions_.height());
    assert(size <= dimensions_.width());
    std::copy(buf, buf + size, get_row(row));
}

template <typename T>
inline void image<T>::set_row(std::size_t row, std::size_t x0, std::size_t x1, pixel_type const* buf)
{
    assert(row < dimensions_.height());
    assert ((x1 - x0) <= dimensions_.width() );
    std::copy(buf, buf + (x1 - x0), get_row(row, x0));
}

template <typename T>
inline double image<T>::get_offset() const
{
    return offset_;
}

template <typename T>
inline void image<T>::set_offset(double set)
{
    offset_ = set;
}

template <typename T>
inline double image<T>::get_scaling() const
{
    return scaling_;
}

template <typename T>
inline void image<T>::set_scaling(double scaling)
{
    if (scaling != 0.0)
    {
        scaling_ = scaling;
        return;
    }
}

template <typename T>
inline bool image<T>::get_premultiplied() const
{
    return premultiplied_alpha_;
}

template <typename T>
inline void image<T>::set_premultiplied(bool set)
{
    premultiplied_alpha_ = set;
}

template <typename T>
inline void image<T>::painted(bool painted)
{
    painted_ = painted;
}

template <typename T>
inline bool image<T>::painted() const
{
    return painted_;
}

template <typename T>
inline image_dtype image<T>::get_dtype()  const
{
    return dtype;
}

} // end ns
