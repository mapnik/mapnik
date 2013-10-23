/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko, Jean-Francois Doyon
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

#include "boost_std_shared_shim.hpp"

// boost
#include <boost/python.hpp>
#include <boost/noncopyable.hpp>
#include <memory>
#include <boost/ptr_container/ptr_vector.hpp>
// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/wkt/wkt_factory.hpp>

namespace  impl {

typedef boost::ptr_vector<mapnik::geometry_type> path_type;

std::shared_ptr<path_type> from_wkt(mapnik::wkt_parser & p, std::string const& wkt)
{
    std::shared_ptr<path_type> paths = std::make_shared<path_type>();
    if (!p.parse(wkt, *paths))
        throw std::runtime_error("Failed to parse WKT");
    return paths;
}

}

void export_wkt_reader()
{
    using mapnik::wkt_parser;
    using namespace boost::python;

    class_<wkt_parser, boost::noncopyable>("WKTReader",init<>())
        .def("read",&impl::from_wkt)
        ;
}
