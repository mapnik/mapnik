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

#include <mapnik/image_any.hpp>

namespace mapnik {

namespace detail {

struct get_bytes_visitor
{
    template <typename T>
    unsigned char* operator()(T & data)
    {
        return data.getBytes();
    }
};

struct get_dtype_visitor
{
    template <typename T>
    image_dtype operator()(T & data)
    {
        return data.get_dtype();
    }
};

struct get_bytes_visitor_const
{
    template <typename T>
    unsigned char const* operator()(T const& data) const
    {
        return data.getBytes();
    }
};

struct get_width_visitor
{
    template <typename T>
    std::size_t operator()(T const& data) const
    {
        return data.width();
    }
};

struct get_height_visitor
{
    template <typename T>
    std::size_t operator()(T const& data) const
    {
        return data.height();
    }
};

struct get_premultiplied_visitor
{
    template <typename T>
    bool operator()(T const& data) const
    {
        return data.get_premultiplied();
    }
};

struct get_painted_visitor
{
    template <typename T>
    bool operator()(T const& data) const
    {
        return data.painted();
    }
};

struct get_any_size_visitor
{
    template <typename T>
    unsigned operator()(T const& data) const
    {
        return data.getSize();
    }
};

struct get_any_row_size_visitor
{
    template <typename T>
    unsigned operator()(T const& data) const
    {
        return data.getRowSize();
    }
};

struct get_offset_visitor
{
    template <typename T>
    double operator() (T const& data) const
    {
        return data.get_offset();
    }
};

struct get_scaling_visitor
{
    template <typename T>
    double operator() (T const& data) const
    {
        return data.get_scaling();
    }
};

struct set_offset_visitor
{
    set_offset_visitor(double val)
        : val_(val) {}
    template <typename T>
    void operator() (T & data)
    {
        data.set_offset(val_);
    }
  private:
    double val_;
};

struct set_scaling_visitor
{
    set_scaling_visitor(double val)
        : val_(val) {}
    template <typename T>
    void operator() (T & data)
    {
        data.set_scaling(val_);
    }
  private:
    double val_;
};

} // namespace detail

image_any::image_any(int width,
                     int height,
                     image_dtype type,
                     bool initialize, 
                     bool premultiplied, 
                     bool painted)
    : image_base(std::move(create_image_any(width, height, type, initialize, premultiplied, painted))) {}

template <typename T>
image_any::image_any(T && data) noexcept
    : image_base(std::move(data)) {}

template image_any::image_any(image_null && data) noexcept;
template image_any::image_any(image_rgba8 && data) noexcept;
template image_any::image_any(image_gray8 && data) noexcept;
template image_any::image_any(image_gray8s && data) noexcept;
template image_any::image_any(image_gray16 && data) noexcept;
template image_any::image_any(image_gray16s && data) noexcept;
template image_any::image_any(image_gray32 && data) noexcept;
template image_any::image_any(image_gray32s && data) noexcept;
template image_any::image_any(image_gray32f && data) noexcept;
template image_any::image_any(image_gray64 && data) noexcept;
template image_any::image_any(image_gray64s && data) noexcept;
template image_any::image_any(image_gray64f && data) noexcept;
template image_any::image_any(image_null const& data) noexcept;
template image_any::image_any(image_rgba8 const& data) noexcept;
template image_any::image_any(image_gray8 const& data) noexcept;
template image_any::image_any(image_gray8s const& data) noexcept;
template image_any::image_any(image_gray16 const& data) noexcept;
template image_any::image_any(image_gray16s const& data) noexcept;
template image_any::image_any(image_gray32 const& data) noexcept;
template image_any::image_any(image_gray32s const& data) noexcept;
template image_any::image_any(image_gray32f const& data) noexcept;
template image_any::image_any(image_gray64 const& data) noexcept;
template image_any::image_any(image_gray64s const& data) noexcept;
template image_any::image_any(image_gray64f const& data) noexcept;

unsigned char const* image_any::getBytes() const
{
    return util::apply_visitor(detail::get_bytes_visitor_const(),*this);
}

unsigned char* image_any::getBytes()
{
    return util::apply_visitor(detail::get_bytes_visitor(),*this);
}

std::size_t image_any::width() const
{
    return util::apply_visitor(detail::get_width_visitor(),*this);
}

std::size_t image_any::height() const
{
    return util::apply_visitor(detail::get_height_visitor(),*this);
}

bool image_any::get_premultiplied() const
{
    return util::apply_visitor(detail::get_premultiplied_visitor(),*this);
}

bool image_any::painted() const
{
    return util::apply_visitor(detail::get_painted_visitor(),*this);
}

unsigned image_any::getSize() const
{
    return util::apply_visitor(detail::get_any_size_visitor(),*this);
}

unsigned image_any::getRowSize() const
{
    return util::apply_visitor(detail::get_any_row_size_visitor(),*this);
}

double image_any::get_offset() const
{
    return util::apply_visitor(detail::get_offset_visitor(),*this);
}

double image_any::get_scaling() const
{
    return util::apply_visitor(detail::get_scaling_visitor(),*this);
}

image_dtype image_any::get_dtype() const
{
    return util::apply_visitor(detail::get_dtype_visitor(),*this);
}

void image_any::set_offset(double val)
{
    util::apply_visitor(detail::set_offset_visitor(val),*this);
}

void image_any::set_scaling(double val)
{
    util::apply_visitor(detail::set_scaling_visitor(val),*this);
}


image_any create_image_any(int width, 
                           int height,
                           image_dtype type, 
                           bool initialize, 
                           bool premultiplied, 
                           bool painted)
{
    switch (type)
    {
      case image_dtype_gray8:
        return image_any(std::move(image_gray8(width, height, initialize, premultiplied, painted)));
      case image_dtype_gray8s:
        return image_any(std::move(image_gray8s(width, height, initialize, premultiplied, painted)));
      case image_dtype_gray16:
        return image_any(std::move(image_gray16(width, height, initialize, premultiplied, painted)));
      case image_dtype_gray16s:
        return image_any(std::move(image_gray16s(width, height, initialize, premultiplied, painted)));
      case image_dtype_gray32:
        return image_any(std::move(image_gray32(width, height, initialize, premultiplied, painted)));
      case image_dtype_gray32s:
        return image_any(std::move(image_gray32s(width, height, initialize, premultiplied, painted)));
      case image_dtype_gray32f:
        return image_any(std::move(image_gray32f(width, height, initialize, premultiplied, painted)));
      case image_dtype_gray64:
        return image_any(std::move(image_gray64(width, height, initialize, premultiplied, painted)));
      case image_dtype_gray64s:
        return image_any(std::move(image_gray64s(width, height, initialize, premultiplied, painted)));
      case image_dtype_gray64f:
        return image_any(std::move(image_gray64f(width, height, initialize, premultiplied, painted)));
      case image_dtype_null:
        return image_any(std::move(image_null()));
      case image_dtype_rgba8:
      case IMAGE_DTYPE_MAX:
      default:
        return image_any(std::move(image_rgba8(width, height, initialize, premultiplied, painted)));
    }
}

} // end mapnik ns
