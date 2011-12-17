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
#include <boost/python/iterator.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/make_shared.hpp>

// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/wkt/wkt_factory.hpp>
#include <mapnik/wkb.hpp>

#include <boost/version.hpp>
#if BOOST_VERSION >= 104700
#include <mapnik/util/geometry_to_wkb.hpp>
#include <mapnik/util/geometry_to_wkt.hpp>
#endif

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

void add_wkt_impl(path_type& p, std::string const& wkt)
{
    bool result = mapnik::from_wkt(wkt , p);
    if (!result) throw std::runtime_error("Failed to parse WKT");
}

void add_wkb_impl(path_type& p, std::string const& wkb)
{
    mapnik::geometry_utils::from_wkb(p, wkb.c_str(), wkb.size());
}

boost::shared_ptr<path_type> from_wkt_impl(std::string const& wkt)
{
    boost::shared_ptr<path_type> paths = boost::make_shared<path_type>();
    bool result = mapnik::from_wkt(wkt, *paths);
    if (!result) throw std::runtime_error("Failed to parse WKT");
    return paths;
}

boost::shared_ptr<path_type> from_wkb_impl(std::string const& wkb)
{
    boost::shared_ptr<path_type> paths = boost::make_shared<path_type>();
    mapnik::geometry_utils::from_wkb(*paths, wkb.c_str(), wkb.size());
    return paths;
}

}

PyObject* to_wkb( geometry_type const& geom)
{
#if BOOST_VERSION >= 104700
    mapnik::util::wkb_buffer_ptr wkb = mapnik::util::to_wkb(geom,mapnik::util::wkbXDR);
    return
#if PY_VERSION_HEX >= 0x03000000
        ::PyBytes_FromStringAndSize
#else
        ::PyString_FromStringAndSize
#endif
        ((const char*)wkb->buffer(),wkb->size());
#else
    std::ostringstream s;
    s << BOOST_VERSION/100000 << "." << BOOST_VERSION/100 % 1000  << "." << BOOST_VERSION % 100;
    throw std::runtime_error("mapnik::to_wkb() requires at least boost 1.47 while your build was compiled against boost " + s.str());
#endif
}

std::string to_wkt( geometry_type const& geom)
{
#if BOOST_VERSION >= 104700
    std::string wkt; // Use Python String directly ?
    bool result = mapnik::util::to_wkt(wkt,geom);
    if (!result) 
    {
        throw std::runtime_error("Generate WKT failed");
    }
    return wkt;
#else
    std::ostringstream s;
    s << BOOST_VERSION/100000 << "." << BOOST_VERSION/100 % 1000  << "." << BOOST_VERSION % 100;
    throw std::runtime_error("mapnik::to_wkt() requires at least boost 1.47 while your build was compiled against boost " + s.str());
#endif
}

std::string to_wkt2( path_type const& geom)
{
#if BOOST_VERSION >= 104700
    std::string wkt; // Use Python String directly ?
    bool result = mapnik::util::to_wkt(wkt,geom);
    if (!result) 
    {
        throw std::runtime_error("Generate WKT failed");
    }
    return wkt;
#else
    std::ostringstream s;
    s << BOOST_VERSION/100000 << "." << BOOST_VERSION/100 % 1000  << "." << BOOST_VERSION % 100;
    throw std::runtime_error("mapnik::to_wkt() requires at least boost 1.47 while your build was compiled against boost " + s.str());
#endif
}


void export_geometry()
{
    using namespace boost::python;

    enum_<mapnik::eGeomType>("GeometryType")
        .value("Point",mapnik::Point)
        .value("LineString",mapnik::LineString)
        .value("Polygon",mapnik::Polygon)
        ;
    
    using mapnik::geometry_type;
    class_<geometry_type, std::auto_ptr<geometry_type>, boost::noncopyable>("Geometry2d",no_init)
        .def("envelope",&geometry_type::envelope)
        // .def("__str__",&geometry_type::to_string)
        .def("type",&geometry_type::type)
        .def("to_wkb",&to_wkb)
        .def("to_wkt",&to_wkt)
        // TODO add other geometry_type methods
        ;

    class_<path_type, boost::shared_ptr<path_type>, boost::noncopyable>("Path")
        .def("__getitem__", getitem_impl,return_value_policy<reference_existing_object>())
        .def("__len__", &path_type::size)
        .def("add_wkt",add_wkt_impl)
        .def("add_wkb",add_wkb_impl)
        .def("to_wkt",&to_wkt2)
        .def("from_wkt",from_wkt_impl)
        .def("from_wkb",from_wkb_impl)
        .staticmethod("from_wkt")
        .staticmethod("from_wkb")
        ;

}
