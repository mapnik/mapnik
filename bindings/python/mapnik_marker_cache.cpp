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

#include <mapnik/utils.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/graphics.hpp>

// boost
#include <boost/make_shared.hpp>
#include <boost/python.hpp>
#include <boost/noncopyable.hpp>

namespace {

bool add_marker_from_image(mapnik::marker_cache & cache, std::string const& key, mapnik::image_32 const& im)
{
    boost::optional<mapnik::image_ptr> imagep(boost::make_shared<mapnik::image_data_32>(im.data()));
    return cache.insert_marker(key,boost::make_shared<mapnik::marker>(imagep));
}

bool add_marker_from_svg(mapnik::marker_cache & cache, std::string const& key, mapnik::svg_storage_type const& svg)
{
    mapnik::svg_path_ptr marker_path(boost::make_shared<mapnik::svg_storage_type>(svg));
    return cache.insert_marker(key,boost::make_shared<mapnik::marker>(marker_path));
}

boost::python::list get_keys(boost::shared_ptr<mapnik::marker_cache> const& cache)
{
    boost::python::list l;
    mapnik::marker_cache::iterator_type itr = cache->begin();
    mapnik::marker_cache::iterator_type end = cache->end();
    for (;itr != end; ++itr)
    {
        l.append(itr->first);
    }
    return l;
}

}

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
        .def("remove",&marker_cache::remove)
        .def("size",&marker_cache::size)
        .def("put",&add_marker_from_image)
        .def("put",&add_marker_from_svg)
        .def("keys",&get_keys)
        ;
}
