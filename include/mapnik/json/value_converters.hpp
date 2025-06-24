/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef MAPNIK_JSON_VALUE_CONVERTERS_HPP
#define MAPNIK_JSON_VALUE_CONVERTERS_HPP

namespace mapnik {
namespace detail {

template<typename T>
struct value_converter
{
    using result_type = T;
    template<typename T1>
    result_type operator()(T1 const& val) const
    {
        return static_cast<result_type>(val);
    }
};

} // namespace detail
} // namespace mapnik

#endif // MAPNIK_JSON_VALUE_CONVERTERS_HPP
