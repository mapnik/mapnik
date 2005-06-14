/* This file is part of python_mapnik (c++/python mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
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

//$Id: mapnik_parameters.cc 17 2005-03-08 23:58:43Z pavlenko $

#include <mapnik.hpp>
#include <boost/python.hpp>
#include <boost/python/detail/api_placeholder.hpp>

using mapnik::Parameter;
using mapnik::Parameters;


void (Parameters::*add1)(const Parameter& param)=&Parameters::add;
void (Parameters::*add2)(const std::string&,const std::string&)=&Parameters::add;

struct parameter_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const Parameter& p)
    {
        using namespace boost::python;
        return boost::python::make_tuple(p.first,p.second);
    }
};

struct parameters_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getstate(const Parameters& p)
    {
        using namespace boost::python;
        dict d;
        Parameters::const_iterator pos=p.begin();
        while(pos!=p.end())
        {
            d[pos->first]=pos->second;
            ++pos;
        }
        return boost::python::make_tuple(d);
    }

    static void setstate(Parameters& p, boost::python::tuple state)
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
        list keys=d.keys();
        for (int i=0;i<len(keys);++i)
        {
            std::string key=extract<std::string>(keys[i]);
            std::string value=extract<std::string>(d[key]);
            p.add(key,value);
        }
    }
};


void export_parameters()
{
    using namespace boost::python;
    class_<Parameter>("parameter",init<std::string,std::string>())
        .def_pickle(parameter_pickle_suite())
        ;

    class_<Parameters>("parameters",init<>())
        .def("add",add1)
        .def("add",add2)
        .def("get",&Parameters::get)
        .def_pickle(parameters_pickle_suite())
        ;
}
