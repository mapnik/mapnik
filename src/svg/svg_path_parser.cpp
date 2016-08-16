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

#include <mapnik/svg/svg_path_parser.hpp>
#include <mapnik/svg/svg_path_grammar_impl.hpp>
// stl
#include <cstring>
#include <string>

namespace mapnik {
namespace svg {

template <typename PathType>
bool parse_path(const char* wkt, PathType& p)
{
    using namespace boost::spirit;
    using iterator_type = const char*;
    using skip_type = ascii::space_type;
    static const svg_path_grammar<iterator_type, PathType, skip_type> g;
    iterator_type first = wkt;
    iterator_type last = wkt + std::strlen(wkt);
    bool status = qi::phrase_parse(first, last, (g)(boost::phoenix::ref(p)), skip_type());
    return (status && (first == last));
}
template bool MAPNIK_DECL parse_path<svg_converter_type>(const char*, svg_converter_type&);

} // namespace svg
} // namespace mapnik
