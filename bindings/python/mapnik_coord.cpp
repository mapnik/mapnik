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
#include <mapnik/config.hpp>
#include "boost_std_shared_shim.hpp"

// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <boost/python.hpp>
#pragma GCC diagnostic pop


// mapnik
#include <mapnik/coord.hpp>

using mapnik::coord;

struct coord_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const coord<double,2>& c)
    {
        using namespace boost::python;
        return boost::python::make_tuple(c.x,c.y);
    }
};

void export_coord()
{
    using namespace boost::python;
    class_<coord<double,2> >("Coord",init<double, double>(
                                 // class docstring is in mapnik/__init__.py, class _Coord
                                 (arg("x"), arg("y")),
                                 "Constructs a new point with the given coordinates.\n")
        )
        .def_pickle(coord_pickle_suite())
        .def_readwrite("x", &coord<double,2>::x,
                       "Gets or sets the x/lon coordinate of the point.\n")
        .def_readwrite("y", &coord<double,2>::y,
                       "Gets or sets the y/lat coordinate of the point.\n")
        .def(self == self) // __eq__
        .def(self + self) // __add__
        .def(self + float())
        .def(float() + self)
        .def(self - self) // __sub__
        .def(self - float())
        .def(self * float()) //__mult__
        .def(float() * self)
        .def(self / float()) // __div__
        ;
}
