/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#ifndef MAPNIK_CONST_RENDERING_BUFFER_HPP
#define MAPNIK_CONST_RENDERING_BUFFER_HPP

#include <mapnik/safe_cast.hpp>

#include "agg_basics.h"

#include <cstdint>

namespace mapnik { namespace util {

// non-mutable rendering_buffer implementation
template <typename T>
struct rendering_buffer
{
    using image_type = T;
    using pixel_type = typename image_type::pixel_type;
    using row_data = agg::const_row_info<uint8_t>;

    rendering_buffer(T const& data)
        : data_(data) {}

    uint8_t const* buf() const { return data_.bytes(); }
    std::size_t width() const { return data_.width();}
    std::size_t height() const { return data_.height();}
    int stride() const { return data_.row_size();}
    uint8_t const* row_ptr(int, int y, unsigned) {return row_ptr(y);}
    uint8_t const* row_ptr(int y) const { return reinterpret_cast<std::uint8_t const*>(data_.get_row(static_cast<std::size_t>(y))); }
    row_data row (int y) const { return row_data(0, safe_cast<int>(data_.width() - 1), row_ptr(y)); }
    image_type const& data_;
};


}}

#endif // MAPNIK_CONST_RENDERING_BUFFER_HPP