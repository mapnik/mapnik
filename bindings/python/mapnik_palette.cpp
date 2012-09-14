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

// boost
#include <boost/python.hpp>
#include <boost/make_shared.hpp>

//mapnik
#include <mapnik/palette.hpp>

static boost::shared_ptr<mapnik::rgba_palette> make_palette( std::string const& palette, std::string const& format )
{
    mapnik::rgba_palette::palette_type type = mapnik::rgba_palette::PALETTE_RGBA;
    if (format == "rgb")
        type = mapnik::rgba_palette::PALETTE_RGB;
    else if (format == "act")
        type = mapnik::rgba_palette::PALETTE_ACT;
    else
        throw std::runtime_error("invalid type passed for mapnik.Palette: must be either rgba, rgb, or act");
    return boost::make_shared<mapnik::rgba_palette>(palette, type);
}

void export_palette ()
{
    using namespace boost::python;

    class_<mapnik::rgba_palette,
        boost::shared_ptr<mapnik::rgba_palette>,
        boost::noncopyable >("Palette",no_init)
        //, init<std::string,std::string>(
        // ( arg("palette"), arg("type")),
        // "Creates a new color palette from a file\n"
        // )
        .def( "__init__", boost::python::make_constructor(make_palette))
        ;
}
