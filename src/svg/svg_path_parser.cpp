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

// mapnik

#include <mapnik/svg/svg_path_parser.hpp>
#include <mapnik/svg/svg_path_grammar_x3.hpp>
// stl
#include <cstring>
#include <string>

namespace mapnik {
namespace svg {

template<typename PathType>
bool parse_path(const char* wkt, PathType& p)
{
    using namespace boost::spirit;
    using iterator_type = char const*;
    iterator_type first = wkt;
    iterator_type last = wkt + std::strlen(wkt);
    bool relative = false;
    using space_type = mapnik::svg::grammar::space_type;
#if BOOST_VERSION >= 106700
    auto const grammar = x3::with<mapnik::svg::grammar::svg_path_tag>(
      p)[x3::with<mapnik::svg::grammar::relative_tag>(relative)[mapnik::svg::grammar::svg_path]];
#else
    auto const grammar = x3::with<mapnik::svg::grammar::svg_path_tag>(
      std::ref(p))[x3::with<mapnik::svg::grammar::relative_tag>(std::ref(relative))[mapnik::svg::grammar::svg_path]];
#endif
    try
    {
        if (!x3::phrase_parse(first, last, grammar, space_type()) || first != last)
        {
            throw std::runtime_error("Failed to parse svg-path");
        }
    } catch (...)
    {
        return false;
    }
    return true;
}

template bool MAPNIK_DECL parse_path<svg_converter_type>(const char*, svg_converter_type&);

} // namespace svg
} // namespace mapnik
