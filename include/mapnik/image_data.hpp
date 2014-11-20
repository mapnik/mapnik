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

#ifndef MAPNIK_IMAGE_DATA_HPP
#define MAPNIK_IMAGE_DATA_HPP

// mapnik
#include <mapnik/global.hpp>

// stl
#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace mapnik
{
template <typename T>
class image_data
{
public:
    using pixel_type = T;

    image_data(std::size_t width, std::size_t height, bool initialize = true)
        : width_(width),
          height_(height),
          pData_((width!=0 && height!=0) ? static_cast<T*>(::operator new(sizeof(T) * width_ * height_)):0),
          owns_data_(true)
    {
        if (pData_ && initialize) std::fill(pData_, pData_ + width_ * height_, 0);
    }

    image_data(std::size_t width, std::size_t height, T * data)
        : width_(width),
          height_(height),
          owns_data_(false),
          pData_(data) {}

    image_data(image_data<T> const& rhs)
        : width_(rhs.width_),
          height_(rhs.height_),
          pData_((rhs.width_!=0 && rhs.height_!=0) ?
                 static_cast<T*>(::operator new(sizeof(T) * rhs.width_ * rhs.height_)) : 0),
          owns_data_(true)
    {
        if (pData_) std::copy(rhs.pData_, rhs.pData_ + rhs.width_* rhs.height_, pData_);
    }

    image_data(image_data<T> && rhs) noexcept
        : width_(rhs.width_),
          height_(rhs.height_),
          pData_(rhs.pData_)
    {
        rhs.width_ = 0;
        rhs.height_ = 0;
        rhs.pData_ = nullptr;
    }

    image_data<T>& operator=(image_data<T> rhs)
    {
        swap(rhs);
        return *this;
    }

    void swap(image_data<T> & rhs)
    {
        std::swap(width_, rhs.width_);
        std::swap(height_, rhs.height_);
        std::swap(pData_, rhs.pData_);
    }

    inline T& operator() (std::size_t i, std::size_t j)
    {
        assert(i<width_ && j<height_);
        return pData_[j * width_ + i];
    }
    inline const T& operator() (std::size_t i,std::size_t j) const
    {
        assert(i < width_ && j < height_);
        return pData_[j * width_ + i];
    }
    inline std::size_t width() const
    {
        return width_;
    }
    inline std::size_t height() const
    {
        return height_;
    }
    inline void set(T const& t)
    {
        std::fill(pData_, pData_ + width_ * height_, t);
    }

    inline const T* getData() const
    {
        return pData_;
    }

    inline T* getData()
    {
        return pData_;
    }

    inline const unsigned char* getBytes() const
    {
        return reinterpret_cast<unsigned char*>(pData_);
    }

    inline unsigned char* getBytes()
    {
        return reinterpret_cast<unsigned char*>(pData_);
    }

    inline const T* getRow(unsigned row) const
    {
        return pData_ + row * width_;
    }

    inline T* getRow(unsigned row)
    {
        return pData_ + row * width_;
    }

    inline void setRow(std::size_t row, T const* buf, std::size_t size)
    {
        assert(row < height_);
        assert(size <= width_);
        std::copy(buf, buf + size, pData_ + row * width_);
    }
    inline void setRow(std::size_t row, std::size_t x0, std::size_t x1, T const* buf)
    {
        std::copy(buf, buf + (x1 - x0), pData_ + row * width_);
    }

    inline ~image_data()
    {
        if (owns_data_)
        {
            ::operator delete(pData_),pData_=0;
        }
    }

private:
    std::size_t width_;
    std::size_t height_;
    T *pData_;
    bool owns_data_;
};

using image_data_32 = image_data<std::uint32_t>;
using image_data_8 = image_data<byte> ;
}

#endif // MAPNIK_IMAGE_DATA_HPP
