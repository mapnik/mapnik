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

#ifndef MAPNIK_IMAGE_VIEW_ANY_HPP
#define MAPNIK_IMAGE_VIEW_ANY_HPP

#include <mapnik/image_view.hpp>
#include <mapnik/image_view_null.hpp>
#include <mapnik/util/variant.hpp>

namespace mapnik {

using image_view_base = util::variant<image_view_null,
                                      image_view_rgba8,
                                      image_view_gray8,
                                      image_view_gray8s,
                                      image_view_gray16,
                                      image_view_gray16s,
                                      image_view_gray32,
                                      image_view_gray32s,
                                      image_view_gray32f,
                                      image_view_gray64,
                                      image_view_gray64s,
                                      image_view_gray64f>;

struct MAPNIK_DECL image_view_any : image_view_base
{
#ifdef _MSC_VER
    // TODO remove this when MSVC doesn't ICE with inherited constructor
    //      https://ci.appveyor.com/project/Mapbox/mapnik/build/1.0.670#L288
    template <typename T>
    image_view_any(T && data)
        noexcept(std::is_nothrow_constructible<image_view_base, T && >::value)
        : image_view_base(std::forward<T>(data)) {}
#else
    // inherit constructors from variant, in particular conversions
    // from alternatives
    using image_view_base::image_view_base;
#endif

    image_view_any() = default;

    // construct from view coordinates and image
    template <typename ImageT>
    image_view_any(std::size_t x, std::size_t y, std::size_t width, std::size_t height,
                   ImageT const& data)
        : image_view_base(image_view<ImageT>(x, y, width, height, data)) {}

    std::size_t width() const;
    std::size_t height() const;
    std::size_t size() const;
    std::size_t row_size() const;
    bool get_premultiplied() const;
    double get_offset() const;
    double get_scaling() const;
    image_dtype get_dtype() const;
};

} // end mapnik ns

#endif // MAPNIK_IMAGE_VIEW_ANY_HPP
