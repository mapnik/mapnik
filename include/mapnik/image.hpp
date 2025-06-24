/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef MAPNIK_IMAGE_HPP
#define MAPNIK_IMAGE_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/pixel_types.hpp>

namespace mapnik {

namespace detail {

struct MAPNIK_DECL buffer
{
    explicit buffer(std::size_t size);
    explicit buffer(unsigned char* data, std::size_t size);
    buffer(buffer&& rhs) noexcept;
    buffer(buffer const& rhs);
    ~buffer();
    buffer& operator=(buffer rhs);
    inline bool operator!() const { return (data_ == nullptr) ? true : false; }
    inline unsigned char* data() { return data_; }
    inline unsigned char const* data() const { return data_; }
    inline std::size_t size() const { return size_; }

  private:
    void swap(buffer& rhs);
    std::size_t size_;
    unsigned char* data_;
    bool owns_;
};

template<std::size_t max_size>
struct image_dimensions
{
    image_dimensions(int width, int height);
    std::size_t width() const;
    std::size_t height() const;

  private:
    std::size_t width_;
    std::size_t height_;
};

} // namespace detail

template<typename T>
class image
{
  public:
    using pixel = T;
    using pixel_type = typename T::type;
    using iterator = pixel_type*;
    using const_iterator = pixel_type const*;
    static constexpr image_dtype dtype = T::id;
    static constexpr std::size_t pixel_size = sizeof(pixel_type);

  private:
    detail::image_dimensions<4294836225> dimensions_;
    detail::buffer buffer_;
    double offset_;
    double scaling_;
    bool premultiplied_alpha_;
    bool painted_;

  public:
    image();
    image(int width, int height, bool initialize = true, bool premultiplied = false, bool painted = false);
    image(int width, int height, unsigned char* data, bool premultiplied = false, bool painted = false);
    image(image<T> const& rhs);
    image(image<T>&& rhs) noexcept;
    image<T>& operator=(image<T> rhs);
    bool operator==(image<T> const& rhs) const;
    bool operator<(image<T> const& rhs) const;

    void swap(image<T>& rhs);
    pixel_type& operator()(std::size_t i, std::size_t j);
    pixel_type const& operator()(std::size_t i, std::size_t j) const;
    std::size_t width() const;
    std::size_t height() const;
    std::size_t size() const;
    std::size_t row_size() const;
    void set(pixel_type const& t);
    pixel_type const* data() const;
    pixel_type* data();
    // simple iterator inteface
    const_iterator begin() const;
    const_iterator end() const;
    iterator begin();
    iterator end();
    //
    unsigned char const* bytes() const;
    unsigned char* bytes();
    pixel_type const* get_row(std::size_t row) const;
    pixel_type const* get_row(std::size_t row, std::size_t x0) const;
    pixel_type* get_row(std::size_t row);
    pixel_type* get_row(std::size_t row, std::size_t x0);
    void set_row(std::size_t row, pixel_type const* buf, std::size_t size);
    void set_row(std::size_t row, std::size_t x0, std::size_t x1, pixel_type const* buf);
    double get_offset() const;
    void set_offset(double set);
    double get_scaling() const;
    void set_scaling(double set);
    bool get_premultiplied() const;
    void set_premultiplied(bool set);
    void painted(bool painted);
    bool painted() const;
    image_dtype get_dtype() const;
};

using image_null = image<null_t>;
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

} // namespace mapnik

#endif // MAPNIK_IMAGE_HPP
