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

#include <mapnik/image_view_any.hpp>

namespace mapnik {

namespace detail {

struct get_view_width_visitor
{
    template <typename T>
    std::size_t operator()(T const& data) const
    {
        return data.width();
    }
};

struct get_view_height_visitor
{
    template <typename T>
    std::size_t operator()(T const& data) const
    {
        return data.height();
    }
};

struct get_view_size_visitor
{
    template <typename T>
    unsigned operator()(T const& data) const
    {
        return data.size();
    }
};

struct get_view_dtype_visitor
{
    template <typename T>
    image_dtype operator()(T const& data) const
    {
        return data.get_dtype();
    }
};

struct get_view_row_size_visitor
{
    template <typename T>
    unsigned operator()(T const& data) const
    {
        return data.row_size();
    }
};

struct get_view_premultiplied_visitor
{
    template <typename T>
    bool operator()(T const& data) const
    {
        return data.get_premultiplied();
    }
};

struct get_view_offset_visitor
{
    template <typename T>
    double operator()(T const& data) const
    {
        return data.get_offset();
    }
};

struct get_view_scaling_visitor
{
    template <typename T>
    double operator()(T const& data) const
    {
        return data.get_scaling();
    }
};

} // end namespace detail

std::size_t image_view_any::width() const
{
    return util::apply_visitor(detail::get_view_width_visitor(),*this);
}

std::size_t image_view_any::height() const
{
    return util::apply_visitor(detail::get_view_height_visitor(),*this);
}

std::size_t image_view_any::size() const
{
    return util::apply_visitor(detail::get_view_size_visitor(),*this);
}

std::size_t image_view_any::row_size() const
{
    return util::apply_visitor(detail::get_view_row_size_visitor(),*this);
}

bool image_view_any::get_premultiplied() const
{
    return util::apply_visitor(detail::get_view_premultiplied_visitor(),*this);
}

double image_view_any::get_offset() const
{
    return util::apply_visitor(detail::get_view_offset_visitor(),*this);
}

double image_view_any::get_scaling() const
{
    return util::apply_visitor(detail::get_view_scaling_visitor(),*this);
}

image_dtype image_view_any::get_dtype() const
{
    return util::apply_visitor(detail::get_view_dtype_visitor(),*this);
}

} // end mapnik ns
