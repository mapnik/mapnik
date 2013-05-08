/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko, Jean-Francois Doyon
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
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/make_shared.hpp>

// mapnik
#include <mapnik/marker.hpp>

namespace {

mapnik::svg_path_ptr open_from_file(std::string const& filename)
{
     return mapnik::read_svg_marker(filename);
}

mapnik::svg_path_ptr fromstring(std::string const& svg)
{
     return mapnik::read_svg_marker(svg,true);
}

}

void export_svg()
{
    using namespace boost::python;
    using mapnik::svg_storage_type;

    class_<svg_storage_type,boost::shared_ptr<svg_storage_type> >("SVG","This class represents an svg object.",no_init)
        .def("width",&svg_storage_type::width)
        .def("height",&svg_storage_type::height)
        .def("extent",make_function(&svg_storage_type::bounding_box,
                           return_value_policy<copy_const_reference>()))
        .def("open",&open_from_file)
        .staticmethod("open")
        .def("fromstring",&fromstring)
        .staticmethod("fromstring")
        ;

}
