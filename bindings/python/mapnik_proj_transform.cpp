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
 
using mapnik::proj_transform;
using mapnik::projection;

struct proj_transform_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const proj_transform& p)
    {
        using namespace boost::python;
        return boost::python::make_tuple(p.source(),p.dest());
    }
};

namespace  {

   mapnik::coord2d forward_transform_c(mapnik::proj_transform& t, mapnik::coord2d const& c)
   {
      double x = c.x;
      double y = c.y;
      double z = 0.0;
      t.forward(x,y,z);
      return mapnik::coord2d(x,y);
   }
   
   mapnik::coord2d backward_transform_c(mapnik::proj_transform& t, mapnik::coord2d const& c)
   {
      double x = c.x;
      double y = c.y;
      double z = 0.0;
      t.backward(x,y,z);
      return mapnik::coord2d(x,y);
   }

   mapnik::Envelope<double> forward_transform_env(mapnik::proj_transform& t, mapnik::Envelope<double> const & box)
   {
      double minx = box.minx();
      double miny = box.miny();
      double maxx = box.maxx();
      double maxy = box.maxy();
      double z = 0.0;
      t.forward(minx,miny,z);
      t.forward(maxx,maxy,z);
      return mapnik::Envelope<double>(minx,miny,maxx,maxy);
   }

   mapnik::Envelope<double> backward_transform_env(mapnik::proj_transform& t, mapnik::Envelope<double> const & box)
   {
      double minx = box.minx();
      double miny = box.miny();
      double maxx = box.maxx();
      double maxy = box.maxy();
      double z = 0.0;
      t.backward(minx,miny,z);
      t.backward(maxx,maxy,z);
      return mapnik::Envelope<double>(minx,miny,maxx,maxy);
   }   
}

void export_proj_transform ()
{
    using namespace boost::python;
    
    class_<proj_transform, boost::noncopyable>("ProjTransform", init< projection const&, projection const& >())
       .def_pickle(proj_transform_pickle_suite())
       .def("forward", forward_transform_c)
       .def("backward",backward_transform_c)
       .def("forward", forward_transform_env)
       .def("backward",backward_transform_env)
       ;
    
}
