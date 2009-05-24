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
#include <mapnik/query.hpp>
#include <mapnik/envelope.hpp>
using mapnik::query;
using mapnik::Envelope;

struct query_pickle_suite : boost::python::pickle_suite
{
   static boost::python::tuple
   getinitargs(const query& q)
   {
      return boost::python::make_tuple(q.get_bbox(),q.resolution());  
   }
};

void export_query()
{
    using namespace boost::python;

    class_<query>("Query", "a spatial query data object", 
		  init<Envelope<double>,double>() )
        .def_pickle(query_pickle_suite())
        .add_property("resolution", &query::resolution)
        .add_property("bbox", make_function(&query::get_bbox,
                                            return_value_policy<copy_const_reference>()) )
        .add_property("property_names", make_function(&query::property_names,
                                                      return_value_policy<copy_const_reference>()) )
        .def("add_property_name", &query::add_property_name);
}



