/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
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

// boost
#include <boost/python.hpp>
#include <boost/python/def.hpp>

// mapnik
#include <mapnik/geometry.hpp>

void export_geometry()
{
   using namespace boost::python;
   using mapnik::geometry2d;
   
   class_<geometry2d, boost::noncopyable>("Geometry2d",no_init)
      .def("envelope",&geometry2d::envelope)
       // .def("__str__",&geometry2d::to_string)
       .def("type",&geometry2d::type)
       // TODO add other geometry2d methods
      ;
}
