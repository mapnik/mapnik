/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
#include <mapnik/svg/svg_transform_grammar.hpp>
// stl
#include <string>
#include <cstring>

namespace mapnik { namespace svg {

    template <typename TransformType>
    bool parse_svg_transform(const char * wkt, TransformType & p)
    {
        using namespace boost::spirit;
        using iterator_type = const char *;
        using skip_type = ascii::space_type;
        // TODO - make it possible for this to be static const
        // by avoiding ctor taking arg - https://github.com/mapnik/mapnik/pull/2231
        svg_transform_grammar<iterator_type,skip_type,TransformType> g(p);
        iterator_type first = wkt;
        iterator_type last =  wkt + std::strlen(wkt);
        return qi::phrase_parse(first, last, g, skip_type());
    }

    template MAPNIK_DECL bool parse_svg_transform<agg::trans_affine>(const char*, agg::trans_affine&);
}}
