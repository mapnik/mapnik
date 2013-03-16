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
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>

// mapnik
#include <mapnik/grid/grid.hpp>
#include "python_grid_utils.hpp"

using namespace boost::python;

// help compiler see template definitions
static dict (*encode)( mapnik::grid const&, std::string const& , bool, unsigned int) = mapnik::grid_encode;

bool painted(mapnik::grid const& grid)
{
    return grid.painted();
}

int get_pixel(mapnik::grid const& grid, int x, int y)
{
    if (x < static_cast<int>(grid.width()) && y < static_cast<int>(grid.height()))
    {
        mapnik::grid::value_type const * row = grid.getRow(y);
        mapnik::grid::value_type const pixel = row[x];
        return pixel;
    }
    PyErr_SetString(PyExc_IndexError, "invalid x,y for grid dimensions");
    boost::python::throw_error_already_set();
    return 0;
}

void export_grid()
{
    class_<mapnik::grid,boost::shared_ptr<mapnik::grid> >(
        "Grid",
        "This class represents a feature hitgrid.",
        init<int,int,std::string,unsigned>(
            ( boost::python::arg("width"), boost::python::arg("height"),boost::python::arg("key")="__id__", boost::python::arg("resolution")=1 ),
            "Create a mapnik.Grid object\n"
            ))
        .def("painted",&painted)
        .def("width",&mapnik::grid::width)
        .def("height",&mapnik::grid::height)
        .def("view",&mapnik::grid::get_view)
        .def("get_pixel",&get_pixel)
        .def("clear",&mapnik::grid::clear)
        .def("encode",encode,
             ( boost::python::arg("encoding")="utf", boost::python::arg("features")=true,boost::python::arg("resolution")=4 ),
             "Encode the grid as as optimized json\n"
            )
        .add_property("key",
                      make_function(&mapnik::grid::get_key,return_value_policy<copy_const_reference>()),
                      &mapnik::grid::set_key,
                      "Get/Set key to be used as unique indentifier for features\n"
                      "The value should either be __id__ to refer to the feature.id()\n"
                      "or some globally unique integer or string attribute field\n"
            )
        ;

}
