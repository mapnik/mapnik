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

#ifndef MAPNIK_SYMBOLIZER_HASH_HPP
#define MAPNIK_SYMBOLIZER_HASH_HPP

// mapnik
#include <mapnik/symbolizer_base.hpp>
#include <mapnik/util/variant.hpp>
// stl
#include <typeinfo>
#include <typeindex>
#include <functional>

namespace mapnik {

struct property_value_hash_visitor
{
    std::size_t operator() (color const& val) const
    {
        return val.rgba();
    }

    std::size_t operator() (font_feature_settings const&) const
    {
        return 0; //FIXME
    }

    std::size_t operator() (transform_type const&) const
    {
        return 0; //FIXME
    }

    std::size_t operator() (enumeration_wrapper const&) const
    {
        return 0; //FIXME
    }

    std::size_t operator() (dash_array const&) const
    {
        return 0; //FIXME
    }

    template <typename T>
    std::size_t operator() (T const& val) const
    {
        return std::hash<T>()(val);
    }
};

struct symbolizer_hash
{
    template <typename T>
    static std::size_t value(T const& sym)
    {
        std::size_t seed = std::hash<std::type_index>()(typeid(sym));
        for (auto const& prop : sym.properties)
        {
            seed ^= std::hash<std::size_t>()(static_cast<std::size_t>(prop.first));
            seed ^= util::apply_visitor(property_value_hash_visitor(), prop.second);
        }
        return seed;
    }
};

struct symbolizer_hash_visitor
{
    template <typename Symbolizer>
    std::size_t operator() (Symbolizer const& sym) const
    {
        return symbolizer_hash::value(sym);
    }
};

} // namespace mapnik


#endif // MAPNIK_SYMBOLIZER_HASH_HPP
