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
class MAPNIK_DECL image_view
{
public:
    using pixel = typename T::pixel;
    using pixel_type = typename T::pixel_type;
    static const image_dtype dtype = T::dtype;
    static constexpr std::size_t pixel_size = sizeof(pixel_type);
    image_view() = delete;
    image_view(unsigned x, unsigned y, unsigned width, unsigned height, T const& data);
    ~image_view();
    
    image_view(image_view<T> const& rhs);
    image_view<T> & operator=(image_view<T> const& rhs);
    bool operator==(image_view<T> const& rhs) const;
    bool operator<(image_view<T> const& rhs) const;

    unsigned x() const;
    unsigned y() const;
    unsigned width() const;
    unsigned height() const;
    const pixel_type& operator() (std::size_t i, std::size_t j) const;
    unsigned getSize() const;
    unsigned getRowSize() const;
    const pixel_type* getRow(unsigned row) const;
    const pixel_type* getRow(unsigned row, std::size_t x0) const;
    T const& data() const;
    bool get_premultiplied() const;
    double get_offset() const;
    double get_scaling() const;
    image_dtype get_dtype() const;

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
