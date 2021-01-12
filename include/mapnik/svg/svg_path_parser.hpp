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

#ifndef MAPNIK_SVG_PATH_PARSER_HPP
#define MAPNIK_SVG_PATH_PARSER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/svg/svg_converter.hpp>
// stl
#include <string>

namespace mapnik {
namespace svg {

template <typename PathType>
bool parse_path(const char* wkt, PathType& p);

template <typename PathType>
bool parse_points(const char* wkt, PathType& p);

template <typename TransformType>
bool parse_svg_transform(const char* wkt, TransformType& tr);

//
extern template bool MAPNIK_DECL parse_path<svg_converter_type>(const char*, svg_converter_type&);
extern template bool MAPNIK_DECL parse_points<svg_converter_type>(const char*, svg_converter_type&);
extern template bool MAPNIK_DECL parse_svg_transform<agg::trans_affine>(const char*, agg::trans_affine&);
}
}



#endif // MAPNIK_SVG_PATH_PARSER_HPP
