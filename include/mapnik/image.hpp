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

#ifndef MAPNIK_IMAGE_DATA_HPP
#define MAPNIK_IMAGE_DATA_HPP

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/pixel_types.hpp>

// stl
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <iostream>

namespace mapnik {

namespace detail {

struct buffer
{
    explicit buffer(std::size_t size)
        : size_(size),
          data_(static_cast<unsigned char*>(size_ != 0 ? ::operator new(size_) : nullptr))
    {}

    buffer(buffer && rhs) noexcept
        : size_(std::move(rhs.size_)),
          data_(std::move(rhs.data_))
    {
        rhs.size_ = 0;
        rhs.data_ = nullptr;
    }

    buffer(buffer const& rhs)
        : size_(rhs.size_),
          data_(static_cast<unsigned char*>(size_ != 0 ? ::operator new(size_) : nullptr))
    {
        if (data_) std::copy(rhs.data_, rhs.data_ + rhs.size_, data_);
    }

    buffer& operator=(buffer rhs)
    {
        swap(rhs);
        return *this;
    }

    void swap(buffer & rhs)
    {
        std::swap(size_, rhs.size_);
        std::swap(data_, rhs.data_);
    }

    inline bool operator!() const { return (data_ == nullptr)? false : true; }
    ~buffer()
    {
        ::operator delete(data_);
    }

    inline unsigned char* data() { return data_; }
    inline unsigned char const* data() const { return data_; }
    inline std::size_t size() const { return size_; }
    std::size_t size_;
    unsigned char* data_;

};

template <std::size_t max_size>
struct image_dimensions
{
    image_dimensions(int width, int height)
        : width_(width),
          height_(height)
    {
        if (width < 0 || static_cast<std::size_t>(width) > max_size) throw std::runtime_error("Invalid width for image dimensions requested");
        if (height < 0 || static_cast<std::size_t>(height) > max_size) throw std::runtime_error("Invalid height for image dimensions requested");
    }

    image_dimensions(image_dimensions const& other) = default;
    image_dimensions(image_dimensions && other) = default;
    image_dimensions& operator= (image_dimensions rhs)
    {
        std::swap(width_, rhs.width_);
        std::swap(height_, rhs.height_);
        return *this;
    }
    std::size_t width() const
    {
        return width_;
    }
    std::size_t height() const
    {
        return height_;
    }
    std::size_t width_;
    std::size_t height_;
};

}

template <typename T, std::size_t max_size = 65535>
class image
{
public:
    using pixel = T;
    using pixel_type = typename T::type;
    static const image_dtype dtype = T::id;
    static constexpr std::size_t pixel_size = sizeof(pixel_type);
private:
    detail::image_dimensions<max_size> dimensions_;
    detail::buffer buffer_;
    pixel_type *pData_;
    double offset_;
    double scaling_;
    bool premultiplied_alpha_;
    bool painted_;
public:
    image(int width, int height, bool initialize = true, bool premultiplied = false, bool painted = false)
        : dimensions_(width, height),
          buffer_(dimensions_.width() * dimensions_.height() * pixel_size),
          pData_(reinterpret_cast<pixel_type*>(buffer_.data())),
          offset_(0.0),
          scaling_(1.0),
          premultiplied_alpha_(premultiplied),
          painted_(painted)
    {
        if (pData_ && initialize) std::fill(pData_, pData_ + dimensions_.width() * dimensions_.height(), 0);
    }

    image(image<T> const& rhs)
        : dimensions_(rhs.dimensions_),
          buffer_(rhs.buffer_),
          pData_(reinterpret_cast<pixel_type*>(buffer_.data())),
          offset_(rhs.offset_),
          scaling_(rhs.scaling_),
          premultiplied_alpha_(rhs.premultiplied_alpha_),
          painted_(rhs.painted_)
    {}

    image(image<T> && rhs) noexcept
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

    image<T>& operator=(image<T> rhs)
    {
        swap(rhs);
        return *this;
    }

    void swap(image<T> & rhs)
    {
        std::swap(dimensions_, rhs.dimensions_);
        std::swap(buffer_, rhs.buffer_);
        std::swap(offset_, rhs.offset_);
        std::swap(scaling_, rhs.scaling_);
        std::swap(premultiplied_alpha_, rhs.premultiplied_alpha_);
        std::swap(painted_, rhs.painted_);
    }

    inline pixel_type& operator() (std::size_t i, std::size_t j)
    {
        assert(i < dimensions_.width() && j < dimensions_.height());
        return pData_[j * dimensions_.width() + i];
    }
    inline const pixel_type& operator() (std::size_t i, std::size_t j) const
    {
        assert(i < dimensions_.width() && j < dimensions_.height());
        return pData_[j * dimensions_.width() + i];
    }
    inline std::size_t width() const
    {
        return dimensions_.width();
    }
    inline std::size_t height() const
    {
        return dimensions_.height();
    }
    inline unsigned getSize() const
    {
        return dimensions_.height() * dimensions_.width() * pixel_size;
    }
    inline unsigned getRowSize() const
    {
        return dimensions_.width() * pixel_size;
    }
    inline void set(pixel_type const& t)
    {
        std::fill(pData_, pData_ + dimensions_.width() * dimensions_.height(), t);
    }

    inline const pixel_type* getData() const
    {
        return pData_;
    }

    inline pixel_type* getData()
    {
        return pData_;
    }

    inline const unsigned char* getBytes() const
    {
        return buffer_.data();
    }

    inline unsigned char* getBytes()
    {
        return buffer_.data();
    }

    inline const pixel_type* getRow(std::size_t row) const
    {
        return pData_ + row * dimensions_.width();
    }

    inline const pixel_type* getRow(std::size_t row, std::size_t x0) const
    {
        return pData_ + row * dimensions_.width() + x0;
    }

    inline pixel_type* getRow(std::size_t row)
    {
        return pData_ + row * dimensions_.width();
    }

    inline pixel_type* getRow(std::size_t row, std::size_t x0)
    {
        return pData_ + row * dimensions_.width() + x0;
    }

    inline void setRow(std::size_t row, pixel_type const* buf, std::size_t size)
    {
        assert(row < dimensions_.height());
        assert(size <= dimensions_.width());
        std::copy(buf, buf + size, pData_ + row * dimensions_.width());
    }
    inline void setRow(std::size_t row, std::size_t x0, std::size_t x1, pixel_type const* buf)
    {
        assert(row < dimensions_.height());
        assert ((x1 - x0) <= dimensions_.width() );
        std::copy(buf, buf + (x1 - x0), pData_ + row * dimensions_.width() + x0);
    }

    inline double get_offset() const
    {
        return offset_;
    }

    inline void set_offset(double set)
    {
        offset_ = set;
    }

    inline double get_scaling() const
    {
        return scaling_;
    }

    inline void set_scaling(double set)
    {
        if (set != 0.0)
        {
            scaling_ = set;
            return;
        }
        std::clog << "Can not set scaling to 0.0, offset not set." << std::endl;
    }

    inline bool get_premultiplied() const
    {
        return premultiplied_alpha_;
    }

    inline void set_premultiplied(bool set)
    {
        premultiplied_alpha_ = set;
    }

    inline void painted(bool painted)
    {
        painted_ = painted;
    }

    inline bool painted() const
    {
        return painted_;
    }

    inline image_dtype get_dtype()  const
    {
        return dtype;
    }
};

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

} // end ns

#endif // MAPNIK_IMAGE_DATA_HPP
