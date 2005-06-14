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

//$Id$

#include "mapnik.hpp"
#include <boost/python.hpp>

void export_datasource_cache()
{
    using mapnik::datasource_cache;
    using mapnik::singleton;
    using mapnik::CreateStatic;
    using namespace boost::python;
    class_<singleton<datasource_cache,CreateStatic>,boost::noncopyable>("singleton",no_init)
        .def("instance",&singleton<datasource_cache,CreateStatic>::instance,
	     return_value_policy<reference_existing_object>())
        .staticmethod("instance")
        ;

    class_<datasource_cache,bases<singleton<datasource_cache,CreateStatic> >,
        boost::noncopyable>("datasource_cache",no_init)
        .def("create",&datasource_cache::create)
        .staticmethod("create")
	.def("register_datasources",&datasource_cache::register_datasources)
	.staticmethod("register_datasources")
        ;
}
