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

// boost
#include <boost/python.hpp>

// mapnik
#include <mapnik/query.hpp>
#include <mapnik/box2d.hpp>

using mapnik::query;
using mapnik::box2d;

namespace python = boost::python;

struct resolution_to_tuple
{
    static PyObject* convert(query::resolution_type const& x)
    {
        python::object tuple(python::make_tuple(x.get<0>(), x.get<1>()));
        return python::incref(tuple.ptr());
    }

    static PyTypeObject const* get_pytype()
    {
        return &PyTuple_Type;
    }
};

void export_query()
{
    using namespace boost::python;

    to_python_converter<query::resolution_type, resolution_to_tuple> ();

    class_<query>("Query", "a spatial query data object",
                  init<box2d<double>,query::resolution_type const&,double>() )
        .def(init<box2d<double> >())
        .add_property("resolution",make_function(&query::resolution,
                                                 return_value_policy<copy_const_reference>()))
        .add_property("bbox", make_function(&query::get_bbox,
                                            return_value_policy<copy_const_reference>()) )
        .add_property("property_names", make_function(&query::property_names,
                                                      return_value_policy<copy_const_reference>()) )
        .def("add_property_name", &query::add_property_name);
}



