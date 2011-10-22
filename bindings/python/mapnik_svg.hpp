/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Robert Coup
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
#ifndef MAPNIK_PYTHON_BINDING_SVG_INCLUDED
#define MAPNIK_PYTHON_BINDING_SVG_INCLUDED

// mapnik
#include <mapnik/symbolizer.hpp>
#include <mapnik/svg/svg_path_parser.hpp>
#include <mapnik/value_error.hpp>

// agg
#include "agg_trans_affine.h"

namespace mapnik {
using namespace boost::python;

template <class T>
const std::string get_svg_transform(T& symbolizer)
{
    return symbolizer.get_transform_string();
}

template <class T>
void set_svg_transform(T& symbolizer, std::string const& transform_wkt)
{
    agg::trans_affine tr;
    if (!mapnik::svg::parse_transform(transform_wkt.c_str(), tr))
    {
        std::stringstream ss;
        ss << "Could not parse transform from '" << transform_wkt << "', expected string like: 'matrix(1, 0, 0, 1, 0, 0)'";
        throw mapnik::value_error(ss.str());    
    }
    mapnik::transform_type matrix;
    tr.store_to(&matrix[0]);
    symbolizer.set_transform(matrix);
}

} // end of namespace mapnik

#endif // MAPNIK_PYTHON_BINDING_SVG_INCLUDED
