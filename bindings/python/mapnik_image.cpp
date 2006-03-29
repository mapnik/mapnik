/* This file is part of python_mapnik (c++/python mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko, Jean-Francois Doyon
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id$

# include <boost/python.hpp>
# include <boost/python/module.hpp>
# include <boost/python/def.hpp>
# include <mapnik.hpp>

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
	;
    
    def("rawdata",&rawdata);
}
