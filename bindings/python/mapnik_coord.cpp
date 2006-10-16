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
//$Id$

#include <boost/python.hpp>
#include <mapnik/coord.hpp>

void export_coord()
{
    using namespace boost::python;
    using mapnik::coord;
    class_<coord<double,2> >("Coord",init<double,double>())
        .def_readwrite("x", &coord<double,2>::x)
        .def_readwrite("y", &coord<double,2>::y)
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
