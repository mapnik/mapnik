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

#include <mapnik/image_any.hpp>

namespace mapnik {

namespace detail {

struct get_bytes_visitor
{
    template<typename T>
    unsigned char* operator()(T& data) const
    {
        return data.bytes();
    }
};

struct get_bytes_visitor_const
{
    template<typename T>
    unsigned char const* operator()(T const& data) const
    {
        return data.bytes();
    }
};

struct get_dtype_visitor
{
    template<typename T>
    image_dtype operator()(T const& data) const
    {
        return data.get_dtype();
    }
};

struct get_width_visitor
{
    template<typename T>
    std::size_t operator()(T const& data) const
    {
        return data.width();
    }
};

struct get_height_visitor
{
    template<typename T>
    std::size_t operator()(T const& data) const
    {
        return data.height();
    }
};

struct get_premultiplied_visitor
{
    template<typename T>
    bool operator()(T const& data) const
    {
        return data.get_premultiplied();
    }
};

struct get_painted_visitor
{
    template<typename T>
    bool operator()(T const& data) const
    {
        return data.painted();
    }
};

struct get_any_size_visitor
{
    template<typename T>
    std::size_t operator()(T const& data) const
    {
        return data.size();
    }
};

struct get_any_row_size_visitor
{
    template<typename T>
    std::size_t operator()(T const& data) const
    {
        return data.row_size();
    }
};

struct get_offset_visitor
{
    template<typename T>
    double operator()(T const& data) const
    {
        return data.get_offset();
    }
};

struct get_scaling_visitor
{
    template<typename T>
    double operator()(T const& data) const
    {
        return data.get_scaling();
    }
};

struct set_offset_visitor
{
    set_offset_visitor(double val)
        : val_(val)
    {}
    template<typename T>
    void operator()(T& data) const
    {
        data.set_offset(val_);
    }

  private:
    double val_;
};

struct set_scaling_visitor
{
    set_scaling_visitor(double val)
        : val_(val)
    {}
    template<typename T>
    void operator()(T& data) const
    {
        data.set_scaling(val_);
    }

  private:
    double val_;
};

} // namespace detail

MAPNIK_DECL
image_any::image_any(int width, int height, image_dtype type, bool initialize, bool premultiplied, bool painted)
    : image_base(std::move(create_image_any(width, height, type, initialize, premultiplied, painted)))
{}

MAPNIK_DECL unsigned char const* image_any::bytes() const
{
    return util::apply_visitor(detail::get_bytes_visitor_const(), *this);
}

MAPNIK_DECL unsigned char* image_any::bytes()
{
    return util::apply_visitor(detail::get_bytes_visitor(), *this);
}

MAPNIK_DECL std::size_t image_any::width() const
{
    return util::apply_visitor(detail::get_width_visitor(), *this);
}

MAPNIK_DECL std::size_t image_any::height() const
{
    return util::apply_visitor(detail::get_height_visitor(), *this);
}

MAPNIK_DECL bool image_any::get_premultiplied() const
{
    return util::apply_visitor(detail::get_premultiplied_visitor(), *this);
}

MAPNIK_DECL bool image_any::painted() const
{
    return util::apply_visitor(detail::get_painted_visitor(), *this);
}

MAPNIK_DECL std::size_t image_any::size() const
{
    return util::apply_visitor(detail::get_any_size_visitor(), *this);
}

MAPNIK_DECL std::size_t image_any::row_size() const
{
    return util::apply_visitor(detail::get_any_row_size_visitor(), *this);
}

MAPNIK_DECL double image_any::get_offset() const
{
    return util::apply_visitor(detail::get_offset_visitor(), *this);
}

MAPNIK_DECL double image_any::get_scaling() const
{
    return util::apply_visitor(detail::get_scaling_visitor(), *this);
}

MAPNIK_DECL image_dtype image_any::get_dtype() const
{
    return util::apply_visitor(detail::get_dtype_visitor(), *this);
}

MAPNIK_DECL void image_any::set_offset(double val)
{
    util::apply_visitor(detail::set_offset_visitor(val), *this);
}

MAPNIK_DECL void image_any::set_scaling(double val)
{
    util::apply_visitor(detail::set_scaling_visitor(val), *this);
}

MAPNIK_DECL image_any
  create_image_any(int width, int height, image_dtype type, bool initialize, bool premultiplied, bool painted)
{
    switch (type)
    {
        case image_dtype_gray8:
            return image_any(image_gray8(width, height, initialize, premultiplied, painted));
        case image_dtype_gray8s:
            return image_any(image_gray8s(width, height, initialize, premultiplied, painted));
        case image_dtype_gray16:
            return image_any(image_gray16(width, height, initialize, premultiplied, painted));
        case image_dtype_gray16s:
            return image_any(image_gray16s(width, height, initialize, premultiplied, painted));
        case image_dtype_gray32:
            return image_any(image_gray32(width, height, initialize, premultiplied, painted));
        case image_dtype_gray32s:
            return image_any(image_gray32s(width, height, initialize, premultiplied, painted));
        case image_dtype_gray32f:
            return image_any(image_gray32f(width, height, initialize, premultiplied, painted));
        case image_dtype_gray64:
            return image_any(image_gray64(width, height, initialize, premultiplied, painted));
        case image_dtype_gray64s:
            return image_any(image_gray64s(width, height, initialize, premultiplied, painted));
        case image_dtype_gray64f:
            return image_any(image_gray64f(width, height, initialize, premultiplied, painted));
        case image_dtype_null:
            return image_any(image_null());
        case image_dtype_rgba8:
        case IMAGE_DTYPE_MAX:
        default:
            return image_any(image_rgba8(width, height, initialize, premultiplied, painted));
    }
}

} // namespace mapnik
