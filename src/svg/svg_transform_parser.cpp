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

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/svg/svg_transform_grammar_impl.hpp>
// stl
#include <string>
#include <cstring>

namespace mapnik {
namespace svg {

template <typename TransformType>
bool parse_svg_transform(const char* wkt, TransformType& tr)
{
    using namespace boost::spirit;
    using iterator_type = const char*;
    using skip_type = ascii::space_type;
    static const svg_transform_grammar<iterator_type, TransformType, skip_type> g;
    iterator_type first = wkt;
    iterator_type last = wkt + std::strlen(wkt);
    return qi::phrase_parse(first, last, (g)(boost::phoenix::ref(tr)), skip_type());
}

template bool MAPNIK_DECL parse_svg_transform<agg::trans_affine>(const char*, agg::trans_affine&);

} // namespace svg
} // namespace mapnik
