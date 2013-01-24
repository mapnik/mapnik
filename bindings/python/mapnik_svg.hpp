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
#include <mapnik/parse_transform.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/value_error.hpp>

namespace mapnik {
using namespace boost::python;

template <class T>
std::string get_svg_transform(T& symbolizer)
{
    return symbolizer.get_image_transform_string();
}

template <class T>
void set_svg_transform(T& symbolizer, std::string const& transform_wkt)
{
    transform_list_ptr trans_expr = mapnik::parse_transform(transform_wkt);
    if (!trans_expr)
    {
        std::stringstream ss;
        ss << "Could not parse transform from '" 
           << transform_wkt 
           << "', expected SVG transform attribute";
        throw mapnik::value_error(ss.str());
    }
    symbolizer.set_image_transform(trans_expr);
}

} // end of namespace mapnik

#endif // MAPNIK_PYTHON_BINDING_SVG_INCLUDED
