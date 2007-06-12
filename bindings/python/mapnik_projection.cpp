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
#include <mapnik/projection.hpp>

namespace {
    mapnik::coord2d forward(mapnik::coord2d const& pt, 
                            mapnik::projection const& prj)
    {
        double x = pt.x;
        double y = pt.y;
        prj.forward(x,y);
        return mapnik::coord2d(x,y);
    }
    
    mapnik::coord2d inverse(mapnik::coord2d const& pt, 
                            mapnik::projection const& prj)
    {
        double x = pt.x;
        double y = pt.y;
        prj.inverse(x,y);
        return mapnik::coord2d(x,y);
    }
    
}

void export_projection ()
{
    using namespace boost::python; 
    using mapnik::projection;
    
    class_<projection>("Projection", init<optional<std::string const&> >())
        .def ("params", make_function(&projection::params,
                                      return_value_policy<copy_const_reference>()))
        .add_property ("geographic",&projection::is_geographic)
        ;
    
    def("forward_",&forward);
    def("inverse_",&inverse);
    
}
