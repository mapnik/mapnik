/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
#include <boost/make_shared.hpp>

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/params.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/value.hpp>

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
            d[pos->first] = pos->second;
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
        boost::python::list keys = d.keys();
        for (int i=0; i<len(keys); ++i)
        {
            std::string key = extract<std::string>(keys[i]);
            object obj = d[key];
            extract<std::string> ex0(obj);
            extract<int> ex1(obj);
            extract<double> ex2(obj);
            extract<UnicodeString> ex3(obj);

            // TODO - this is never hit - we need proper python string -> std::string to get invoked here
            if (ex0.check())
            {
                p[key] = ex0();
            }
            else if (ex1.check())
            {
                p[key] = ex1();
            }
            else if (ex2.check())
            {
                p[key] = ex2();
            }
            else if (ex3.check())
            {
                std::string buffer;
                mapnik::to_utf8(ex3(),buffer);
                p[key] = buffer;
            }
            else
            {
                MAPNIK_LOG_DEBUG(bindings) << "parameters_pickle_suite: Could not unpickle key=" << key;
            }
        }
    }
};


mapnik::value_holder get_params_by_key1(mapnik::parameters const& p, std::string const& key)
{
    parameters::const_iterator pos = p.find(key);
    if (pos != p.end())
    {
        // will be auto-converted to proper python type by `mapnik_params_to_python`
        return pos->second;
    }
    return mapnik::value_null();
}

mapnik::value_holder get_params_by_key2(mapnik::parameters const& p, std::string const& key)
{
    parameters::const_iterator pos = p.find(key);
    if (pos == p.end())
    {
        PyErr_SetString(PyExc_KeyError, key.c_str());
        boost::python::throw_error_already_set();
    }
    // will be auto-converted to proper python type by `mapnik_params_to_python`
    return pos->second;
}

mapnik::parameter get_params_by_index(mapnik::parameters const& p, int index)
{
    if (index < 0 || static_cast<unsigned>(index) > p.size())
    {
        PyErr_SetString(PyExc_IndexError, "Index is out of range");
        throw boost::python::error_already_set();
    }

    parameters::const_iterator itr = p.begin();
    parameters::const_iterator end = p.end();

    int idx = 0;
    while (itr != end)
    {
        if (idx == index)
        {
            return *itr;
        }
        ++idx;
        ++itr;
    }
    PyErr_SetString(PyExc_IndexError, "Index is out of range");
    throw boost::python::error_already_set();
}

void add_parameter(mapnik::parameters & p, mapnik::parameter const& param)
{
    p[param.first] = param.second;
}

mapnik::value_holder get_param(mapnik::parameter const& p, int index)
{
    if (index == 0)
    {
        return p.first;
    }
    else if (index == 1)
    {
        return p.second;
    }
    else
    {
        PyErr_SetString(PyExc_IndexError, "Index is out of range");
        throw boost::python::error_already_set();
    }
}

boost::shared_ptr<mapnik::parameter> create_parameter_from_string(std::string const& key, std::string const& value)
{
    return boost::make_shared<mapnik::parameter>(key,mapnik::value_holder(value));
}

boost::shared_ptr<mapnik::parameter> create_parameter_from_int(std::string const& key, int value)
{
    return boost::make_shared<mapnik::parameter>(key,mapnik::value_holder(value));
}

boost::shared_ptr<mapnik::parameter> create_parameter_from_float(std::string const& key, double value)
{
    return boost::make_shared<mapnik::parameter>(key,mapnik::value_holder(value));
}


void export_parameters()
{
    using namespace boost::python;
    class_<parameter,boost::shared_ptr<parameter> >("Parameter",no_init)
        .def("__init__", make_constructor(create_parameter_from_string),
             "Create a mapnik.Parameter from a pair of values, the first being a string\n"
             "and the second being either a string, and integer, or a float")
        .def("__init__", make_constructor(create_parameter_from_int),
             "Create a mapnik.Parameter from a pair of values, the first being a string\n"
             "and the second being either a string, and integer, or a float")
        .def("__init__", make_constructor(create_parameter_from_float),
             "Create a mapnik.Parameter from a pair of values, the first being a string\n"
             "and the second being either a string, and integer, or a float")
        .def_pickle(parameter_pickle_suite())
        .def("__getitem__",get_param)
        ;

    class_<parameters>("Parameters",init<>())
        .def_pickle(parameters_pickle_suite())
        .def("get",get_params_by_key1)
        .def("__getitem__",get_params_by_key2)
        .def("__getitem__",get_params_by_index)
        .def("__len__",&parameters::size)
        .def("append",add_parameter)
        .def("iteritems",iterator<parameters>())
        ;
}
