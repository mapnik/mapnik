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
#include <mapnik/datasource_cache.hpp>

namespace  {

using namespace boost::python;

boost::shared_ptr<mapnik::datasource> create_datasource(const dict& d)
{
    bool bind=true;
    mapnik::parameters params;
    boost::python::list keys=d.keys();
    for (int i=0; i<len(keys); ++i)
    {
        std::string key = extract<std::string>(keys[i]);
        object obj = d[key];

        if (key == "bind")
        {
            bind = extract<bool>(obj)();
            continue;
        }

        extract<std::string> ex0(obj);
        extract<int> ex1(obj);
        extract<double> ex2(obj);

        if (ex0.check())
        {
            params[key] = ex0();
        }
        else if (ex1.check())
        {
            params[key] = ex1();
        }
        else if (ex2.check())
        {
            params[key] = ex2();
        }
    }

    return mapnik::datasource_cache::instance().create(params, bind);
}

void register_datasources(std::string const& path)
{
    mapnik::datasource_cache::instance().register_datasources(path);
}

std::vector<std::string> plugin_names()
{
    return mapnik::datasource_cache::instance().plugin_names();
}

std::string plugin_directories()
{
    return mapnik::datasource_cache::instance().plugin_directories();
}

}

void export_datasource_cache()
{
    using mapnik::datasource_cache;
    class_<datasource_cache,
           boost::noncopyable>("DatasourceCache",no_init)
        .def("create",&create_datasource)
        .staticmethod("create")
        .def("register_datasources",&register_datasources)
        .staticmethod("register_datasources")
        .def("plugin_names",&plugin_names)
        .staticmethod("plugin_names")
        .def("plugin_directories",&plugin_directories)
        .staticmethod("plugin_directories")
        ;
}
