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

#ifndef MAPNIK_IMAGE_DATA_ANY_HPP
#define MAPNIK_IMAGE_DATA_ANY_HPP

#include <mapnik/image_data.hpp>
#include <mapnik/image_view.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/util/variant.hpp>

namespace mapnik {

struct image_data_null
{
    unsigned char const* getBytes() const { return nullptr; }
    unsigned char* getBytes() { return nullptr;}
    std::size_t width() const { return 0; }
    std::size_t height() const { return 0; }
};

using image_data_base = util::variant<image_data_null, 
                                      image_data_rgba8, 
                                      image_data_gray8, 
                                      image_data_gray16, 
                                      image_data_gray32f,
                                      image_32,
                                      image_view<image_data_rgba8>,
                                      image_view<image_data_gray8>,
                                      image_view<image_data_gray16>,
                                      image_view<image_data_gray32f>>;

namespace detail {

/*struct get_bytes_visitor : util::static_visitor<unsigned char*>
{
    template <typename T>
    unsigned char* operator()(T & data)
    {
        return data.getBytes();
    }
};*/

struct get_bytes_visitor_const : util::static_visitor<unsigned char const*>
{
    template <typename T>
    unsigned char const* operator()(T const& data) const
    {
        return data.getBytes();
    }
};

struct get_width_visitor : util::static_visitor<std::size_t>
{
    template <typename T>
    std::size_t operator()(T const& data) const
    {
        return data.width();
    }
};

struct get_height_visitor : util::static_visitor<std::size_t>
{
    template <typename T>
    std::size_t operator()(T const& data) const
    {
        return data.height();
    }
};

} // namespace detail

struct image_data_any : image_data_base
{
    image_data_any() = default;

    template <typename T>
    image_data_any(T && data) noexcept
        : image_data_base(std::move(data)) {}

    unsigned char const* getBytes() const
    {
        return util::apply_visitor(detail::get_bytes_visitor_const(),*this);
    }

    /*unsigned char* getBytes()
    {
        return util::apply_visitor(detail::get_bytes_visitor(),*this);
    }*/

    std::size_t width() const
    {
        return util::apply_visitor(detail::get_width_visitor(),*this);
    }

    std::size_t height() const
    {
        return util::apply_visitor(detail::get_height_visitor(),*this);
    }
};

}

#endif // MAPNIK_IMAGE_DATA_ANY_HPP
