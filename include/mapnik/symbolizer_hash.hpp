/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

#include <mapnik/map.hpp>
#include <mapnik/feature_type_style.hpp>

#include <boost/functional/hash.hpp>
#include <boost/variant/static_visitor.hpp>

namespace mapnik {

struct symbolizer_hash
{
    template <typename T>
    static std::size_t value(T const& sym)
    {
        std::size_t seed = 0;
        return seed;
    }
    // specialisation for polygon_symbolizer
    static std::size_t value(polygon_symbolizer const& sym)
    {
        std::size_t seed = Polygon;
        boost::hash_combine(seed, sym.get_fill().rgba());
        boost::hash_combine(seed, sym.get_opacity());
        return seed;
    }

    // specialisation for line_symbolizer
    static std::size_t value(line_symbolizer const& sym)
    {
        std::size_t seed = LineString;
        boost::hash_combine(seed, sym.get_stroke().get_color().rgba());
        boost::hash_combine(seed, sym.get_stroke().get_width());
        boost::hash_combine(seed, sym.get_stroke().get_opacity());
        boost::hash_combine(seed, static_cast<int>(sym.get_stroke().get_line_cap()));
        boost::hash_combine(seed, static_cast<int>(sym.get_stroke().get_line_join()));
        return seed;
    }
};

}

#endif // MAPNIK_SYMBOLIZER_HASH_HPP
