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

#ifndef MAPNIK_IMAGE_IMPL_HPP
#define MAPNIK_IMAGE_IMPL_HPP

// mapnik
#include <mapnik/image.hpp>

// stl
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <iostream>

namespace mapnik {

template <typename T, std::size_t max_size>
image<T,max_size>::image()
    : dimensions_(0,0),
      buffer_(0),
      pData_(nullptr),
      offset_(0.0),
      scaling_(1.0),
      premultiplied_alpha_(false),
      painted_(false) 
{}

template <typename T, std::size_t max_size>
image<T,max_size>::image(int width, int height, bool initialize, bool premultiplied, bool painted)
        : dimensions_(width, height),
          buffer_(dimensions_.width() * dimensions_.height() * pixel_size),
          pData_(reinterpret_cast<pixel_type*>(buffer_.data())),
          offset_(0.0),
          scaling_(1.0),
          premultiplied_alpha_(premultiplied),
          painted_(painted)
{
    if (pData_ && initialize)
    {
        std::fill(pData_, pData_ + dimensions_.width() * dimensions_.height(), 0);
    }
}

template <typename T, std::size_t max_size>
image<T,max_size>::image(image<T> const& rhs)
        : dimensions_(rhs.dimensions_),
          buffer_(rhs.buffer_),
          pData_(reinterpret_cast<pixel_type*>(buffer_.data())),
          offset_(rhs.offset_),
          scaling_(rhs.scaling_),
          premultiplied_alpha_(rhs.premultiplied_alpha_),
          painted_(rhs.painted_)
{}

template <typename T, std::size_t max_size>
image<T,max_size>::image(image<T> && rhs) noexcept
        : dimensions_(std::move(rhs.dimensions_)),
          buffer_(std::move(rhs.buffer_)),
          pData_(reinterpret_cast<pixel_type*>(buffer_.data())),
          offset_(rhs.offset_),
          scaling_(rhs.scaling_),
          premultiplied_alpha_(rhs.premultiplied_alpha_),
          painted_(rhs.painted_)
{
    rhs.dimensions_ = { 0, 0 };
    rhs.pData_ = nullptr;
}

template <typename T, std::size_t max_size>
image<T>& image<T,max_size>::operator=(image<T> rhs)
{
    swap(rhs);
    return *this;
}

template <typename T, std::size_t max_size>
void image<T,max_size>::swap(image<T> & rhs)
{
    std::swap(dimensions_, rhs.dimensions_);
    std::swap(buffer_, rhs.buffer_);
    std::swap(offset_, rhs.offset_);
    std::swap(scaling_, rhs.scaling_);
    std::swap(premultiplied_alpha_, rhs.premultiplied_alpha_);
    std::swap(painted_, rhs.painted_);
}

template <typename T, std::size_t max_size>
inline typename image<T,max_size>::pixel_type& image<T,max_size>::operator() (std::size_t i, std::size_t j)
{
    assert(i < dimensions_.width() && j < dimensions_.height());
    return pData_[j * dimensions_.width() + i];
}

template <typename T, std::size_t max_size>
inline const typename image<T,max_size>::pixel_type& image<T,max_size>::operator() (std::size_t i, std::size_t j) const
{
    assert(i < dimensions_.width() && j < dimensions_.height());
    return pData_[j * dimensions_.width() + i];
}

template <typename T, std::size_t max_size>
inline std::size_t image<T,max_size>::width() const
{
    return dimensions_.width();
}

template <typename T, std::size_t max_size>
inline std::size_t image<T,max_size>::height() const
{
    return dimensions_.height();
}

template <typename T, std::size_t max_size>
inline unsigned image<T,max_size>::getSize() const
{
    return dimensions_.height() * dimensions_.width() * pixel_size;
}
    
template <typename T, std::size_t max_size>
inline unsigned image<T,max_size>::getRowSize() const
{
    return dimensions_.width() * pixel_size;
}

template <typename T, std::size_t max_size>
inline void image<T,max_size>::set(pixel_type const& t)
{
    std::fill(pData_, pData_ + dimensions_.width() * dimensions_.height(), t);
}

template <typename T, std::size_t max_size>
inline const typename image<T,max_size>::pixel_type* image<T,max_size>::getData() const
{
    return pData_;
}

template <typename T, std::size_t max_size>
inline typename image<T,max_size>::pixel_type* image<T,max_size>::getData()
{
    return pData_;
}

template <typename T, std::size_t max_size>
inline const unsigned char* image<T,max_size>::getBytes() const
{
    return buffer_.data();
}

template <typename T, std::size_t max_size>
inline unsigned char* image<T,max_size>::getBytes()
{
    return buffer_.data();
}

template <typename T, std::size_t max_size>
inline const typename image<T,max_size>::pixel_type* image<T,max_size>::getRow(std::size_t row) const
{
    return pData_ + row * dimensions_.width();
}

template <typename T, std::size_t max_size>
inline const typename image<T,max_size>::pixel_type* image<T,max_size>::getRow(std::size_t row, std::size_t x0) const
{
    return pData_ + row * dimensions_.width() + x0;
}

template <typename T, std::size_t max_size>
inline typename image<T,max_size>::pixel_type* image<T,max_size>::getRow(std::size_t row)
{
    return pData_ + row * dimensions_.width();
}

template <typename T, std::size_t max_size>
inline typename image<T,max_size>::pixel_type* image<T,max_size>::getRow(std::size_t row, std::size_t x0)
{
    return pData_ + row * dimensions_.width() + x0;
}

template <typename T, std::size_t max_size>
inline void image<T,max_size>::setRow(std::size_t row, pixel_type const* buf, std::size_t size)
{
    assert(row < dimensions_.height());
    assert(size <= dimensions_.width());
    std::copy(buf, buf + size, pData_ + row * dimensions_.width());
}

template <typename T, std::size_t max_size>
inline void image<T,max_size>::setRow(std::size_t row, std::size_t x0, std::size_t x1, pixel_type const* buf)
{
    assert(row < dimensions_.height());
    assert ((x1 - x0) <= dimensions_.width() );
    std::copy(buf, buf + (x1 - x0), pData_ + row * dimensions_.width() + x0);
}

template <typename T, std::size_t max_size>
inline double image<T,max_size>::get_offset() const
{
    return offset_;
}

template <typename T, std::size_t max_size>
inline void image<T,max_size>::set_offset(double set)
{
    offset_ = set;
}

template <typename T, std::size_t max_size>
inline double image<T,max_size>::get_scaling() const
{
    return scaling_;
}

template <typename T, std::size_t max_size>
inline void image<T,max_size>::set_scaling(double set)
{
    if (set != 0.0)
    {
        scaling_ = set;
        return;
    }
    std::clog << "Can not set scaling to 0.0, offset not set." << std::endl;
}

template <typename T, std::size_t max_size>
inline bool image<T,max_size>::get_premultiplied() const
{
    return premultiplied_alpha_;
}

template <typename T, std::size_t max_size>
inline void image<T,max_size>::set_premultiplied(bool set)
{
    premultiplied_alpha_ = set;
}

template <typename T, std::size_t max_size>
inline void image<T,max_size>::painted(bool painted)
{
    painted_ = painted;
}

template <typename T, std::size_t max_size>
inline bool image<T,max_size>::painted() const
{
    return painted_;
}

template <typename T, std::size_t max_size>
inline image_dtype image<T,max_size>::get_dtype()  const
{
    return dtype;
}

/*
using image_rgba8 = image<rgba8_t>;
using image_gray8 = image<gray8_t>;
using image_gray8s = image<gray8s_t>;
using image_gray16 = image<gray16_t>;
using image_gray16s = image<gray16s_t>;
using image_gray32 = image<gray32_t>;
using image_gray32s = image<gray32s_t>;
using image_gray32f = image<gray32f_t>;
using image_gray64 = image<gray64_t>;
using image_gray64s = image<gray64s_t>;
using image_gray64f = image<gray64f_t>;
*/
} // end ns

#endif // MAPNIK_IMAGE_IMPL_HPP
