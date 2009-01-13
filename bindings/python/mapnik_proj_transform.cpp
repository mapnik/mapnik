/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2009 Artem Pavlenko
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

// mapnik
#include <mapnik/proj_transform.hpp>
// boost
#include <boost/python.hpp>

namespace  {

   mapnik::coord2d forward_transform(mapnik::proj_transform& t, mapnik::coord2d const& c)
   {
      double x = c.x;
      double y = c.y;
      double z = 0.0;
      t.forward(x,y,z);
      return mapnik::coord2d(x,y);
   }
   
   mapnik::coord2d backward_transform(mapnik::proj_transform& t, mapnik::coord2d const& c)
   {
      double x = c.x;
      double y = c.y;
      double z = 0.0;
      t.backward(x,y,z);
      return mapnik::coord2d(x,y);
   }
}

void export_proj_transform ()
{
    using namespace boost::python; 
    using mapnik::proj_transform;
    using mapnik::projection;
    class_<proj_transform, boost::noncopyable>("ProjTransform", init< projection const&, projection const& >())
       .def("forward", forward_transform)
       .def("backward",backward_transform)
       ;
    
}
