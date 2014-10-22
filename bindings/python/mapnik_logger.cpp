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

#include <boost/python.hpp>
#include <boost/noncopyable.hpp>
#pragma GCC diagnostic pop

#include <mapnik/debug.hpp>
#include <mapnik/utils.hpp>
#include "mapnik_enumeration.hpp"

void export_logger()
{
    using mapnik::logger;
    using mapnik::singleton;
    using mapnik::CreateStatic;
    using namespace boost::python;

    class_<singleton<logger,CreateStatic>,boost::noncopyable>("Singleton",no_init)
        .def("instance",&singleton<logger,CreateStatic>::instance,
             return_value_policy<reference_existing_object>())
        .staticmethod("instance")
        ;

    enum_<mapnik::logger::severity_type>("severity_type")
        .value("Debug", logger::debug)
        .value("Warn", logger::warn)
        .value("Error", logger::error)
        .value("None", logger::none)
        ;

    class_<logger,bases<singleton<logger,CreateStatic> >,
        boost::noncopyable>("logger",no_init)
        .def("get_severity", &logger::get_severity)
        .def("set_severity", &logger::set_severity)
        .def("get_object_severity", &logger::get_object_severity)
        .def("set_object_severity", &logger::set_object_severity)
        .def("clear_object_severity", &logger::clear_object_severity)
        .def("get_format", &logger::get_format)
        .def("set_format", &logger::set_format)
        .def("str", &logger::str)
        .def("use_file", &logger::use_file)
        .def("use_console", &logger::use_console)
        .staticmethod("get_severity")
        .staticmethod("set_severity")
        .staticmethod("get_object_severity")
        .staticmethod("set_object_severity")
        .staticmethod("clear_object_severity")
        .staticmethod("get_format")
        .staticmethod("set_format")
        .staticmethod("str")
        .staticmethod("use_file")
        .staticmethod("use_console")
        ;
}
