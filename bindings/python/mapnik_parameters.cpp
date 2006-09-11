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
//$Id: mapnik_parameters.cpp 17 2005-03-08 23:58:43Z pavlenko $

#include <boost/python.hpp>
#include <boost/python/detail/api_placeholder.hpp>
#include <params.hpp>

using mapnik::parameter;
using mapnik::parameters;

struct parameter_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const parameter& p)
    {
        using namespace boost::python;
        return boost::python::make_tuple(p.first,p.second);
    }
};

struct parameters_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getstate(const parameters& p)
    {
        using namespace boost::python;
        dict d;
        parameters::const_iterator pos=p.begin();
        while(pos!=p.end())
        {
            d[pos->first]=pos->second;
            ++pos;
        }
        return boost::python::make_tuple(d);
    }

    static void setstate(parameters& p, boost::python::tuple state)
    {
        using namespace boost::python;
        if (len(state) != 1)
        {
            PyErr_SetObject(PyExc_ValueError,
			    ("expected 1-item tuple in call to __setstate__; got %s"
			     % state).ptr()
			    );
            throw_error_already_set();
        }
        dict d = extract<dict>(state[0]);
        boost::python::list keys=d.keys();
        for (int i=0;i<len(keys);++i)
        {
            std::string key=extract<std::string>(keys[i]);
            std::string value=extract<std::string>(d[key]);
            p[key] = value;
        }
    }
};


void export_parameters()
{
    using namespace boost::python;
    class_<parameter>("Parameter",init<std::string,std::string>())
        .def_pickle(parameter_pickle_suite())
        ;

    class_<parameters>("Parameters",init<>())
        //.def("add",add1)
        //.def("add",add2)
        .def("get",&parameters::get)
        .def_pickle(parameters_pickle_suite())
        ;
}
