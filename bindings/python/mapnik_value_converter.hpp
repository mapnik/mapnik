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
#ifndef MAPNIK_PYTHON_BINDING_VALUE_CONVERTER_INCLUDED
#define MAPNIK_PYTHON_BINDING_VALUE_CONVERTER_INCLUDED

// boost
#include <boost/python.hpp>
#include <boost/implicit_cast.hpp>

namespace boost { namespace python {

    struct value_converter : public boost::static_visitor<PyObject*>
    {
        PyObject * operator() (int val) const
        {
#if PY_VERSION_HEX >= 0x03000000
            return ::PyLong_FromLong(val);
#else
            return ::PyInt_FromLong(val);
#endif
        }

        PyObject * operator() (double val) const
        {
            return ::PyFloat_FromDouble(val);
        }

        PyObject * operator() (bool val) const
        {
            return ::PyBool_FromLong(val);
        }

        PyObject * operator() (std::string const& s) const
        {
            return ::PyUnicode_DecodeUTF8(s.c_str(),implicit_cast<ssize_t>(s.length()),0);
        }

        PyObject * operator() (UnicodeString const& s) const
        {
            std::string buffer;
            mapnik::to_utf8(s,buffer);
            return ::PyUnicode_DecodeUTF8(buffer.c_str(),implicit_cast<ssize_t>(buffer.length()),0);
        }

        PyObject * operator() (mapnik::value_null const& /*s*/) const
        {
            Py_RETURN_NONE;
        }
    };


    struct mapnik_value_to_python
    {
        static PyObject* convert(mapnik::value const& v)
        {
            return boost::apply_visitor(value_converter(),v.base());
        }

    };

    struct mapnik_param_to_python
    {
        static PyObject* convert(mapnik::value_holder const& v)
        {
            return boost::apply_visitor(value_converter(),v);
        }
    };


    }
}

#endif // MAPNIK_PYTHON_BINDING_VALUE_CONVERTER_INCLUDED
