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

#ifndef MAPNIK_VALUE_HASH_HPP
#define MAPNIK_VALUE_HASH_HPP

// mapnik
#include <mapnik/util/variant.hpp>
#include <mapnik/value_types.hpp>

// stl
#include <functional>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <unicode/unistr.h>
#pragma GCC diagnostic pop

namespace mapnik { namespace detail {

template <class T>
inline void hash_combine(std::size_t & seed, T const& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

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
std::size_t mapnik_hash_value(T const& val)
{
    std::size_t seed = util::apply_visitor(detail::value_hasher(), val);
    detail::hash_combine(seed, val.which());
    return seed;
}

} // namespace mapnik

#endif // MAPNIK_VALUE_HASH_HPP
