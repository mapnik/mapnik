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
#include <boost/noncopyable.hpp>

#include <mapnik/utils.hpp>
#include <mapnik/marker_cache.hpp>

void export_marker_cache()
{
    using mapnik::marker_cache;
    using mapnik::singleton;
    using mapnik::CreateUsingNew;
    using namespace boost::python;
    class_<singleton<marker_cache,CreateUsingNew>,boost::noncopyable>("Singleton",no_init)
        .def("instance",&singleton<marker_cache,CreateUsingNew>::instance,
             return_value_policy<reference_existing_object>())
        .staticmethod("instance")
        ;

    class_<marker_cache,bases<singleton<marker_cache,CreateUsingNew> >,
        boost::noncopyable>("MarkerCache",no_init)
        .def("clear",&marker_cache::clear)
        .def("size",&marker_cache::size)
        //.staticmethod("size")
        ;
}
