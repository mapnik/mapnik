/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
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

// boost
#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/exception_translator.hpp>
#include <boost/python/manage_new_object.hpp>
// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/wkt/wkt_factory.hpp>

namespace {

using mapnik::from_wkt;
using mapnik::geometry_type;

geometry_type * make_from_wkt(std::string const& wkt)
{
    std::pair<bool,geometry_type*> result = from_wkt(wkt);
    if (result.first)
    {
        return result.second;
    }
    throw std::runtime_error("Failed to parse WKT");
}

}

void export_geometry()
{
    using namespace boost::python;
    
    enum_<mapnik::eGeomType>("GeometryType")
        .value("Point",mapnik::Point)
        .value("LineString",mapnik::LineString)
        .value("Polygon",mapnik::Polygon)
        .value("MultiPoint",mapnik::MultiPoint)
        .value("MultiLineString",mapnik::MultiLineString)
        .value("MultiPolygon",mapnik::MultiPolygon)
        ;
    
    using mapnik::geometry_type;
    class_<geometry_type, std::auto_ptr<geometry_type>,boost::noncopyable>("Geometry2d",no_init)
        // factory method 
        .def("from_wkt",make_from_wkt,return_value_policy<manage_new_object>())
        .staticmethod("from_wkt")
        .def("envelope",&geometry_type::envelope)
        // .def("__str__",&geometry_type::to_string)
        .def("type",&geometry_type::type)
        .def("area",&geometry_type::area)
        // TODO add other geometry_type methods
        ;
}
