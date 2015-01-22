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

// mapnik
#include <mapnik/image_convert.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/image_any.hpp>
 
namespace mapnik
{

namespace detail
{

template <typename T0>
struct visitor_convert
{
    using dst_type = typename T0::pixel_type;
    template <typename T1>
    T0 operator() (T1 const& src)
    {
        T0 dst(src.width(), src.height());
        for (unsigned y = 0; y < dst.height(); ++y)
        {
            for (unsigned x = 0; x < dst.width(); ++x)
            {
                dst(x,y) = static_cast<dst_type>(src(x,y));
            }
        }
        return T0(std::move(dst));
    }
};

} // end detail ns

template <typename T1, typename T2>
MAPNIK_DECL T2 convert_image(T1 const& data)
{
    detail::visitor_convert<T2> visit;
    return visit(data);
}

} // end mapnik ns
