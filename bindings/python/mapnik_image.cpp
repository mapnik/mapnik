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
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <mapnik/graphics.hpp>

using mapnik::Image32;
using namespace boost::python;

PyObject* rawdata( Image32 const& im)
{
    int size = im.width() * im.height() * 4;
    return ::PyString_FromStringAndSize((const char*)im.raw_data(),size);
}

void export_image()
{
    using namespace boost::python;
    class_<Image32>("Image","This class represents a 32 bit image.",init<int,int>())
	.def("width",&Image32::width)
	.def("height",&Image32::height)
        .add_property("background",make_function
                      (&Image32::getBackground,return_value_policy<copy_const_reference>()),
                       &Image32::setBackground, "The background color of the image.")
	;    
    def("rawdata",&rawdata);
}
