/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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
#include <mapnik/config.hpp>
#include <mapnik/svg/svg_transform_grammar_x3_def.hpp>
// stl
#include <string>
#include <cstring>

namespace mapnik { namespace svg {

template <typename Transform>
bool parse_svg_transform(const char* wkt, Transform& tr)
{
    using namespace boost::spirit;
    using iterator_type = char const*;
    iterator_type first = wkt;
    iterator_type last = wkt + std::strlen(wkt);
    using space_type = mapnik::svg::grammar::space_type;
    auto const grammar = x3::with<mapnik::svg::grammar::svg_transform_tag>(tr)
          [mapnik::svg::svg_transform_grammar()];

    try
    {
        if (!x3::phrase_parse(first, last, grammar, space_type())
            || first != last)
        {
            throw std::runtime_error("Failed to parse svg-transform");
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

template bool MAPNIK_DECL parse_svg_transform<agg::trans_affine>(const char*, agg::trans_affine&);

} // namespace svg
} // namespace mapnik
