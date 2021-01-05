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

#ifndef MAPNIK_VALUE_HASH_HPP
#define MAPNIK_VALUE_HASH_HPP

// mapnik
#include <mapnik/util/variant.hpp>
#include <mapnik/value/types.hpp>
// stl
#include <functional>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <unicode/unistr.h>

MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

namespace detail {

struct value_hasher
{
    std::size_t operator() (value_null val) const
    {
        return hash_value(val);
    }

    std::size_t operator() (value_unicode_string const& val) const
    {
        return static_cast<std::size_t>(val.hashCode());
    }

    template <class T>
    std::size_t operator()(T const& val) const
    {
        std::hash<T> hasher;
        return hasher(val);
    }
};

} // namespace  detail

template <typename T>
std::size_t value_hash(T const& val)
{
    return util::apply_visitor(detail::value_hasher(), val);
}

} // namespace mapnik

#endif // MAPNIK_VALUE_HASH_HPP
