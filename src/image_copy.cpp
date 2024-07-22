/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

// mapnik
#include <mapnik/image_copy.hpp>
#include <mapnik/image.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/safe_cast.hpp>

namespace mapnik {

namespace detail {

template<typename T0>
struct visitor_image_copy
{
    using dst_type = typename T0::pixel_type;

    T0 operator()(image_null const&) const { throw std::runtime_error("Can not cast a null image"); }

    T0 operator()(T0 const& src) { return T0(src); }

    template<typename T1>
    T0 operator()(T1 const& src) const
    {
        T0 dst(safe_cast<int>(src.width()), safe_cast<int>(src.height()), false);
        for (std::size_t y = 0; y < dst.height(); ++y)
        {
            for (std::size_t x = 0; x < dst.width(); ++x)
            {
                dst(x, y) = safe_cast<dst_type>(src(x, y));
            }
        }
        return T0(std::move(dst));
    }
};

template<typename T0>
struct visitor_image_copy_so
{
    using dst_type = typename T0::pixel_type;

    visitor_image_copy_so(double offset, double scaling)
        : offset_(offset)
        , scaling_(scaling)
    {}

    T0 operator()(image_null const&) { throw std::runtime_error("Can not cast a null image"); }

    T0 operator()(T0 const& src) const
    {
        if (offset_ == src.get_offset() && scaling_ == src.get_scaling())
        {
            return T0(src);
        }
        else
        {
            T0 dst(src);
            dst.set_scaling(scaling_);
            dst.set_offset(offset_);
            return T0(std::move(dst));
        }
    }

    template<typename T1>
    T0 operator()(T1 const& src) const
    {
        double src_offset = src.get_offset();
        double src_scaling = src.get_scaling();
        T0 dst(safe_cast<int>(src.width()), safe_cast<int>(src.height()), false);
        dst.set_scaling(scaling_);
        dst.set_offset(offset_);
        for (std::size_t y = 0; y < dst.height(); ++y)
        {
            for (std::size_t x = 0; x < dst.width(); ++x)
            {
                double scaled_src_val = (safe_cast<double>(src(x, y)) * src_scaling) + src_offset;
                double dst_val = (scaled_src_val - offset_) / scaling_;
                dst(x, y) = safe_cast<dst_type>(dst_val);
            }
        }
        return T0(std::move(dst));
    }

  private:
    double offset_;
    double scaling_;
};

} // namespace detail

template<typename T>
MAPNIK_DECL T image_copy(image_any const& data, double offset, double scaling)
{
    if (offset == 0.0 && scaling == 1.0 && data.get_offset() == 0.0 && data.get_scaling() == 1.0)
    {
        return util::apply_visitor(detail::visitor_image_copy<T>(), data);
    }
    else
    {
        return util::apply_visitor(detail::visitor_image_copy_so<T>(offset, scaling), data);
    }
}

template<typename T>
MAPNIK_DECL T image_copy(image_rgba8 const& data, double offset, double scaling)
{
    if (offset == 0.0 && scaling == 1.0 && data.get_offset() == 0.0 && data.get_scaling() == 1.0)
    {
        detail::visitor_image_copy<T> visit;
        return visit(data);
    }
    else
    {
        detail::visitor_image_copy_so<T> visit(offset, scaling);
        return visit(data);
    }
}

template<typename T>
MAPNIK_DECL T image_copy(image_gray8 const& data, double offset, double scaling)
{
    if (offset == 0.0 && scaling == 1.0 && data.get_offset() == 0.0 && data.get_scaling() == 1.0)
    {
        detail::visitor_image_copy<T> visit;
        return visit(data);
    }
    else
    {
        detail::visitor_image_copy_so<T> visit(offset, scaling);
        return visit(data);
    }
}

template<typename T>
MAPNIK_DECL T image_copy(image_gray8s const& data, double offset, double scaling)
{
    if (offset == 0.0 && scaling == 1.0 && data.get_offset() == 0.0 && data.get_scaling() == 1.0)
    {
        detail::visitor_image_copy<T> visit;
        return visit(data);
    }
    else
    {
        detail::visitor_image_copy_so<T> visit(offset, scaling);
        return visit(data);
    }
}

template<typename T>
MAPNIK_DECL T image_copy(image_gray16 const& data, double offset, double scaling)
{
    if (offset == 0.0 && scaling == 1.0 && data.get_offset() == 0.0 && data.get_scaling() == 1.0)
    {
        detail::visitor_image_copy<T> visit;
        return visit(data);
    }
    else
    {
        detail::visitor_image_copy_so<T> visit(offset, scaling);
        return visit(data);
    }
}

template<typename T>
MAPNIK_DECL T image_copy(image_gray16s const& data, double offset, double scaling)
{
    if (offset == 0.0 && scaling == 1.0 && data.get_offset() == 0.0 && data.get_scaling() == 1.0)
    {
        detail::visitor_image_copy<T> visit;
        return visit(data);
    }
    else
    {
        detail::visitor_image_copy_so<T> visit(offset, scaling);
        return visit(data);
    }
}

template<typename T>
MAPNIK_DECL T image_copy(image_gray32 const& data, double offset, double scaling)
{
    if (offset == 0.0 && scaling == 1.0 && data.get_offset() == 0.0 && data.get_scaling() == 1.0)
    {
        detail::visitor_image_copy<T> visit;
        return visit(data);
    }
    else
    {
        detail::visitor_image_copy_so<T> visit(offset, scaling);
        return visit(data);
    }
}

template<typename T>
MAPNIK_DECL T image_copy(image_gray32s const& data, double offset, double scaling)
{
    if (offset == 0.0 && scaling == 1.0 && data.get_offset() == 0.0 && data.get_scaling() == 1.0)
    {
        detail::visitor_image_copy<T> visit;
        return visit(data);
    }
    else
    {
        detail::visitor_image_copy_so<T> visit(offset, scaling);
        return visit(data);
    }
}

template<typename T>
MAPNIK_DECL T image_copy(image_gray32f const& data, double offset, double scaling)
{
    if (offset == 0.0 && scaling == 1.0 && data.get_offset() == 0.0 && data.get_scaling() == 1.0)
    {
        detail::visitor_image_copy<T> visit;
        return visit(data);
    }
    else
    {
        detail::visitor_image_copy_so<T> visit(offset, scaling);
        return visit(data);
    }
}

template<typename T>
MAPNIK_DECL T image_copy(image_gray64 const& data, double offset, double scaling)
{
    if (offset == 0.0 && scaling == 1.0 && data.get_offset() == 0.0 && data.get_scaling() == 1.0)
    {
        detail::visitor_image_copy<T> visit;
        return visit(data);
    }
    else
    {
        detail::visitor_image_copy_so<T> visit(offset, scaling);
        return visit(data);
    }
}

template<typename T>
MAPNIK_DECL T image_copy(image_gray64s const& data, double offset, double scaling)
{
    if (offset == 0.0 && scaling == 1.0 && data.get_offset() == 0.0 && data.get_scaling() == 1.0)
    {
        detail::visitor_image_copy<T> visit;
        return visit(data);
    }
    else
    {
        detail::visitor_image_copy_so<T> visit(offset, scaling);
        return visit(data);
    }
}

template<typename T>
MAPNIK_DECL T image_copy(image_gray64f const& data, double offset, double scaling)
{
    if (offset == 0.0 && scaling == 1.0 && data.get_offset() == 0.0 && data.get_scaling() == 1.0)
    {
        detail::visitor_image_copy<T> visit;
        return visit(data);
    }
    else
    {
        detail::visitor_image_copy_so<T> visit(offset, scaling);
        return visit(data);
    }
}

MAPNIK_DECL image_any image_copy(image_any const& data, image_dtype type, double offset, double scaling)
{
    switch (type)
    {
        case image_dtype_rgba8:
            return image_any(image_copy<image_rgba8>(data, offset, scaling));
        case image_dtype_gray8:
            return image_any(image_copy<image_gray8>(data, offset, scaling));
        case image_dtype_gray8s:
            return image_any(image_copy<image_gray8s>(data, offset, scaling));
        case image_dtype_gray16:
            return image_any(image_copy<image_gray16>(data, offset, scaling));
        case image_dtype_gray16s:
            return image_any(image_copy<image_gray16s>(data, offset, scaling));
        case image_dtype_gray32:
            return image_any(image_copy<image_gray32>(data, offset, scaling));
        case image_dtype_gray32s:
            return image_any(image_copy<image_gray32s>(data, offset, scaling));
        case image_dtype_gray32f:
            return image_any(image_copy<image_gray32f>(data, offset, scaling));
        case image_dtype_gray64:
            return image_any(image_copy<image_gray64>(data, offset, scaling));
        case image_dtype_gray64s:
            return image_any(image_copy<image_gray64s>(data, offset, scaling));
        case image_dtype_gray64f:
            return image_any(image_copy<image_gray64f>(data, offset, scaling));
        case image_dtype_null:
            throw std::runtime_error("Can not cast a null image");
        case IMAGE_DTYPE_MAX:
        default:
            throw std::runtime_error("Can not cast unknown type");
    }
    throw std::runtime_error("Unknown image type passed");
}

} // namespace mapnik
