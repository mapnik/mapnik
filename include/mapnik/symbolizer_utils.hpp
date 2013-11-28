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

#ifndef MAPNIK_SYMBOLIZER_UTILS_HPP
#define MAPNIK_SYMBOLIZER_UTILS_HPP

// mapnik
#include <mapnik/symbolizer.hpp>
// boost
#include <boost/variant/apply_visitor.hpp>

namespace mapnik {

template <typename Symbolizer>
struct symbolizer_traits
{
    static char const* name() { return "Unknown";}
};

template<>
struct symbolizer_traits<point_symbolizer>
{
    static char const* name() { return "PointSymbolizer";}
};

template<>
struct symbolizer_traits<line_symbolizer>
{
    static char const* name() { return "LineSymbolizer";}
};

template<>
struct symbolizer_traits<polygon_symbolizer>
{
    static char const* name() { return "PolygonSymbolizer";}
};

template<>
struct symbolizer_traits<text_symbolizer>
{
    static char const* name() { return "TextSymbolizer";}
};

template<>
struct symbolizer_traits<line_pattern_symbolizer>
{
    static char const* name() { return "LinePatternSymbolizer";}
};

template<>
struct symbolizer_traits<polygon_pattern_symbolizer>
{
    static char const* name() { return "PolygonPatternSymbolizer";}
};

template<>
struct symbolizer_traits<markers_symbolizer>
{
    static char const* name() { return "MarkersSymbolizer";}
};

template<>
struct symbolizer_traits<shield_symbolizer>
{
    static char const* name() { return "ShieldSymbolizer";}
};

template<>
struct symbolizer_traits<raster_symbolizer>
{
    static char const* name() { return "RasterSymbolizer";}
};

template<>
struct symbolizer_traits<building_symbolizer>
{
    static char const* name() { return "BuildingSymbolizer";}
};

template<>
struct symbolizer_traits<debug_symbolizer>
{
    static char const* name() { return "DebugSymbolizer";}
};

// symbolizer name impl
namespace detail {

struct symbolizer_name_impl : public boost::static_visitor<std::string>
{
public:
    template <typename Symbolizer>
    std::string operator () (Symbolizer const& sym) const
    {
        boost::ignore_unused_variable_warning(sym);
        return symbolizer_traits<Symbolizer>::name();
    }
};
}

std::string symbolizer_name(symbolizer const& sym)
{
    std::string type = boost::apply_visitor( detail::symbolizer_name_impl(), sym);
    return type;
}

};

#endif // MAPNIK_SYMBOLIZER_UTILS_HPP
