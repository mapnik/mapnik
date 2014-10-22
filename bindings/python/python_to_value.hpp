/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#ifndef MAPNIK_PYTHON_BINDING_PYTHON_TO_VALUE
#define MAPNIK_PYTHON_BINDING_PYTHON_TO_VALUE

// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <boost/python.hpp>
#pragma GCC diagnostic pop

// mapnik
#include <mapnik/value.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/attribute.hpp>


namespace mapnik {

    static mapnik::attributes dict2attr(boost::python::dict const& d)
    {
        using namespace boost::python;
        mapnik::attributes vars;
        mapnik::transcoder tr_("utf8");
        boost::python::list keys=d.keys();
        for (int i=0; i < len(keys); ++i)
        {
            std::string key = extract<std::string>(keys[i]);
            object obj = d[key];
            if (PyUnicode_Check(obj.ptr()))
            {
                PyObject* temp = PyUnicode_AsUTF8String(obj.ptr());
                if (temp)
                {
    #if PY_VERSION_HEX >= 0x03000000
                    char* c_str = PyBytes_AsString(temp);
    #else
                    char* c_str = PyString_AsString(temp);
    #endif
                    vars[key] = tr_.transcode(c_str);
                    Py_DecRef(temp);
                }
                continue;
            }

            extract<std::string> ex0(obj);
            if (ex0.check())
            {
                vars[key] = tr_.transcode(ex0().c_str());
                continue;
            }
            extract<mapnik::value_integer> ex2(obj);
            if (ex2.check())
            {
                vars[key] = ex2();
                continue;
            }
            extract<double> ex3(obj);
            if (ex3.check())
            {
                vars[key] = ex3();
                continue;
            }
            extract<mapnik::value_bool> ex1(obj);
            if (ex1.check())
            {
                vars[key] = ex1();
                continue;
            }
        }
        return vars;
    }
}

#endif // MAPNIK_PYTHON_BINDING_PYTHON_TO_VALUE
