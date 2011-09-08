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
//#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/iterator.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/wkt/wkt_factory.hpp>

namespace {

using mapnik::from_wkt;
using mapnik::geometry_type;

typedef boost::ptr_vector<geometry_type> path_type;

geometry_type const& getitem_impl(path_type & p, int key)
{
    if (key >=0 && key < p.size())
        return p[key];
    PyErr_SetString(PyExc_IndexError, "Index is out of range");
    throw boost::python::error_already_set();
}

void from_wkt_impl(path_type& p, std::string const& wkt)
{
    bool result = mapnik::from_wkt(wkt, p);
    if (!result) throw std::runtime_error("Failed to parse WKT");
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
        .def("envelope",&geometry_type::envelope)
        // .def("__str__",&geometry_type::to_string)
        .def("type",&geometry_type::type)
        .def("area",&geometry_type::area)
        // TODO add other geometry_type methods
        ;
    
    class_<path_type,boost::noncopyable>("Path")
        .def("__getitem__", getitem_impl,return_value_policy<reference_existing_object>())
        .def("from_wkt",from_wkt_impl)

        ;
    
}
