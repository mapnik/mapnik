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

//$Id$

#include <boost/python.hpp>
#include <mapnik/datasource_cache.hpp>

void export_datasource_cache()
{
    using mapnik::datasource_cache;
    using mapnik::singleton;
    using mapnik::CreateStatic;
    using namespace boost::python;
    class_<singleton<datasource_cache,CreateStatic>,boost::noncopyable>("Singleton",no_init)
        .def("instance",&singleton<datasource_cache,CreateStatic>::instance,
             return_value_policy<reference_existing_object>())
        .staticmethod("instance")
        ;
    
    class_<datasource_cache,bases<singleton<datasource_cache,CreateStatic> >,
        boost::noncopyable>("DatasourceCache",no_init)
        .def("create",&datasource_cache::create)
        .staticmethod("create")
        .def("register_datasources",&datasource_cache::register_datasources)
        .staticmethod("register_datasources")
        .def("plugin_names",&datasource_cache::plugin_names)
        .staticmethod("plugin_names")        
        ;
}
