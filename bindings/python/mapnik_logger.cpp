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

#include <boost/python.hpp>
#include <mapnik/debug.hpp>

using mapnik::logger::severity;
using mapnik::logger::format;
using mapnik::logger::output;

void set_severity(const severity::type& s)
{
    severity::set(s);
}

severity::type get_severity()
{
    return severity::get();
}

void set_object_severity(const std::string& object_name, const severity::type& s)
{
    severity::set_object(object_name, s);
}

severity::type get_object_severity(const std::string& object_name)
{
    return severity::get_object(object_name);
}


void export_logger()
{
    using namespace boost::python;

    enum_<mapnik::logger::severity::type>("SeverityType")
        .value("Info", severity::info)
        .value("Debug", severity::debug)
        .value("Warn", severity::warn)
        .value("Error", severity::error)
        .value("Fatal", severity::fatal)
        .value("None", severity::none)
        ;

/*
    using mapnik::singleton;
    using mapnik::CreateStatic;
    using namespace boost::python;

    class_<singleton<severity,CreateStatic>,boost::noncopyable>("Singleton",no_init)
        .def("instance",&singleton<severity,CreateStatic>::instance,
             return_value_policy<reference_existing_object>())
        .staticmethod("instance")
        ;

    class_<severity,bases<singleton<severity,CreateStatic> >,
        boost::noncopyable>("Severity",no_init)
        .def("get",&severity::get)
        .def("set",&severity::set)
        .def("get_object",&severity::get_object)
        .def("set_object",&severity::set_object)
        .staticmethod("get")
        .staticmethod("set")
        .staticmethod("get_object")
        .staticmethod("set_object")
        ;
*/

    def("set_severity", &set_severity,
        "\n"
        "Set global logger severity.\n"
        "\n"
        "Usage:\n"
        ">>> from mapnik import SeverityType, set_severity\n"
        ">>> set_severity(SeverityType.None)\n"
        ">>> set_severity(SeverityType.Info)\n"
        ">>> set_severity(SeverityType.Debug)\n"
        ">>> set_severity(SeverityType.Warn)\n"
        ">>> set_severity(SeverityType.Error)\n"
        ">>> set_severity(SeverityType.Fatal)\n"
        );

    def("get_severity", &get_severity,
        "\n"
        "Get global logger severity.\n"
        "\n"
        "Usage:\n"
        ">>> from mapnik import get_severity\n"
        ">>> get_severity()\n"
        );

    def("set_object_severity", &set_object_severity,
        "\n"
        "Set logger severity for a single object.\n"
        "\n"
        "Usage:\n"
        ">>> from mapnik import SeverityType, set_object_severity\n"
        ">>> set_object_severity('ogr', SeverityType.None)\n"
        ">>> set_object_severity('gdal', SeverityType.Info)\n"
        ">>> set_object_severity('cairo_renderer', SeverityType.Debug)\n"
        ">>> set_object_severity('agg_renderer', SeverityType.Warn)\n"
        ">>> set_object_severity('bindings', SeverityType.Error)\n"
        );

    def("get_object_severity", &get_object_severity,
        "\n"
        "Get logger severity for a single object.\n"
        "\n"
        "Usage:\n"
        ">>> from mapnik import get_object_severity"
        ">>> get_object_severity('ogr')\n"
        );

}
