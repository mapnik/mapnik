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

// boost
#include "boost_std_shared_shim.hpp"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include "python_to_value.hpp"
#include <boost/python.hpp>
#pragma GCC diagnostic pop

// mapnik
#include <mapnik/query.hpp>
#include <mapnik/box2d.hpp>

#include <string>
#include <set>

using mapnik::query;
using mapnik::box2d;

namespace python = boost::python;

struct resolution_to_tuple
{
    static PyObject* convert(query::resolution_type const& x)
    {
        python::object tuple(python::make_tuple(std::get<0>(x), std::get<1>(x)));
        return python::incref(tuple.ptr());
    }

    static PyTypeObject const* get_pytype()
    {
        return &PyTuple_Type;
    }
};

struct names_to_list
{
    static PyObject* convert(std::set<std::string> const& names)
    {
        boost::python::list l;
        for ( std::string const& name : names )
        {
            l.append(name);
        }
        return python::incref(l.ptr());
    }

    static PyTypeObject const* get_pytype()
    {
        return &PyList_Type;
    }
};

namespace {

    void set_variables(mapnik::query & q, boost::python::dict const& d)
    {
        mapnik::attributes vars = mapnik::dict2attr(d);
        q.set_variables(vars);
    }
}

void export_query()
{
    using namespace boost::python;

    to_python_converter<query::resolution_type, resolution_to_tuple> ();
    to_python_converter<std::set<std::string>, names_to_list> ();

    class_<query>("Query", "a spatial query data object",
                  init<box2d<double>,query::resolution_type const&,double>() )
        .def(init<box2d<double> >())
        .add_property("resolution",make_function(&query::resolution,
                                                 return_value_policy<copy_const_reference>()))
        .add_property("bbox", make_function(&query::get_bbox,
                                            return_value_policy<copy_const_reference>()) )
        .add_property("property_names", make_function(&query::property_names,
                                                      return_value_policy<copy_const_reference>()) )
        .def("add_property_name", &query::add_property_name)
        .def("set_variables",&set_variables);
}
