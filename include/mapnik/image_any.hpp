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

#ifndef MAPNIK_IMAGE_ANY_HPP
#define MAPNIK_IMAGE_ANY_HPP

#include <mapnik/image.hpp>
#include <mapnik/image_null.hpp>
#include <mapnik/util/variant.hpp>

namespace mapnik {

using image_base = util::variant<image_null,
                                 image_rgba8,
                                 image_gray8,
                                 image_gray8s,
                                 image_gray16,
                                 image_gray16s,
                                 image_gray32,
                                 image_gray32s,
                                 image_gray32f,
                                 image_gray64,
                                 image_gray64s,
                                 image_gray64f>;


struct MAPNIK_DECL image_any : image_base
{
    image_any() = default;

    image_any(int width,
              int height,
              image_dtype type = image_dtype_rgba8,
              bool initialize = true,
              bool premultiplied = false,
              bool painted = false);

    template <typename T>
        image_any(T && _data)
        noexcept(std::is_nothrow_constructible<image_base, T && >::value)
        : image_base(std::forward<T>(_data)) {}

    unsigned char const* bytes() const;
    unsigned char* bytes();
    std::size_t width() const;
    std::size_t height() const;
    bool get_premultiplied() const;
    bool painted() const;
    std::size_t size() const;
    std::size_t row_size() const;
    double get_offset() const;
    double get_scaling() const;
    image_dtype get_dtype() const;
    void set_offset(double val);
    void set_scaling(double val);
};

MAPNIK_DECL image_any create_image_any(int width,
                                       int height,
                                       image_dtype type = image_dtype_rgba8,
                                       bool initialize = true,
                                       bool premultiplied = false,
                                       bool painted = false);

} // end mapnik ns

#endif // MAPNIK_IMAGE_ANY_HPP
