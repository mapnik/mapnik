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

#include "boost_std_shared_shim.hpp"

// boost
#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/exception_translator.hpp>
#include <boost/python/manage_new_object.hpp>
#include <boost/python/iterator.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/noncopyable.hpp>


// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/wkt/wkt_factory.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/json/geometry_parser.hpp>
#include <mapnik/json/geojson_generator.hpp>

#include <boost/version.hpp>
#include <mapnik/util/geometry_to_wkb.hpp>
#include <mapnik/util/geometry_to_wkt.hpp>
#include <mapnik/util/geometry_to_svg.hpp>

// stl
#include <stdexcept>

namespace {

using mapnik::from_wkt;
using mapnik::geometry_type;

typedef boost::ptr_vector<geometry_type> path_type;

geometry_type const& getitem_impl(path_type & p, int key)
{
    if (key >=0 && key < static_cast<int>(p.size()))
        return p[key];
    PyErr_SetString(PyExc_IndexError, "Index is out of range");
    throw boost::python::error_already_set();
}

void add_wkt_impl(path_type& p, std::string const& wkt)
{
    if (!mapnik::from_wkt(wkt , p))
        throw std::runtime_error("Failed to parse WKT");
}

void add_wkb_impl(path_type& p, std::string const& wkb)
{
    if (!mapnik::geometry_utils::from_wkb(p, wkb.c_str(), wkb.size()))
        throw std::runtime_error("Failed to parse WKB");
}

void add_geojson_impl(path_type& p, std::string const& json)
{
    if (!mapnik::json::from_geojson(json, p))
        throw std::runtime_error("Failed to parse geojson geometry");
}

std::shared_ptr<path_type> from_wkt_impl(std::string const& wkt)
{
    std::shared_ptr<path_type> paths = std::make_shared<path_type>();
    if (!mapnik::from_wkt(wkt, *paths))
        throw std::runtime_error("Failed to parse WKT");
    return paths;
}

std::shared_ptr<path_type> from_wkb_impl(std::string const& wkb)
{
    std::shared_ptr<path_type> paths = std::make_shared<path_type>();
    if (!mapnik::geometry_utils::from_wkb(*paths, wkb.c_str(), wkb.size()))
        throw std::runtime_error("Failed to parse WKB");
    return paths;
}

std::shared_ptr<path_type> from_geojson_impl(std::string const& json)
{
    std::shared_ptr<path_type> paths = std::make_shared<path_type>();
    if (! mapnik::json::from_geojson(json, *paths))
        throw std::runtime_error("Failed to parse geojson geometry");
    return paths;
}

mapnik::box2d<double> envelope_impl(path_type & p)
{
    mapnik::box2d<double> b;
    bool first = true;
    for (mapnik::geometry_type const& geom : p)
    {
        if (first)
        {
            b = geom.envelope();
            first=false;
        }
        else
        {
            b.expand_to_include(geom.envelope());
        }
    }
    return b;
}

}

inline std::string boost_version()
{
    std::ostringstream s;
    s << BOOST_VERSION/100000 << "." << BOOST_VERSION/100 % 1000  << "." << BOOST_VERSION % 100;
    return s.str();
}

PyObject* to_wkb( geometry_type const& geom, mapnik::util::wkbByteOrder byte_order)
{
    mapnik::util::wkb_buffer_ptr wkb = mapnik::util::to_wkb(geom,byte_order);
    if (wkb)
    {
        return
#if PY_VERSION_HEX >= 0x03000000
            ::PyBytes_FromStringAndSize
#else
            ::PyString_FromStringAndSize
#endif
            ((const char*)wkb->buffer(),wkb->size());
    }
    else
    {
        Py_RETURN_NONE;
    }
}

PyObject* to_wkb2( path_type const& p, mapnik::util::wkbByteOrder byte_order)
{
    mapnik::util::wkb_buffer_ptr wkb = mapnik::util::to_wkb(p,byte_order);
    if (wkb)
    {
        return
#if PY_VERSION_HEX >= 0x03000000
            ::PyBytes_FromStringAndSize
#else
            ::PyString_FromStringAndSize
#endif
            ((const char*)wkb->buffer(),wkb->size());
    }
    else
    {
        Py_RETURN_NONE;
    }
}

std::string to_wkt( geometry_type const& geom)
{
    std::string wkt; // Use Python String directly ?
    bool result = mapnik::util::to_wkt(wkt,geom);
    if (!result)
    {
        throw std::runtime_error("Generate WKT failed");
    }
    return wkt;
}

std::string to_wkt2( path_type const& geom)
{
    std::string wkt; // Use Python String directly ?
    bool result = mapnik::util::to_wkt(wkt,geom);
    if (!result)
    {
        throw std::runtime_error("Generate WKT failed");
    }
    return wkt;
}

std::string to_geojson( path_type const& geom)
{
    std::string json;
    mapnik::json::geometry_generator g;
    if (!g.generate(json,geom))
    {
        throw std::runtime_error("Failed to generate GeoJSON");
    }
    return json;
}

std::string to_svg( geometry_type const& geom)
{
    std::string svg; // Use Python String directly ?
    bool result = mapnik::util::to_svg(svg,geom);
    if (!result)
    {
        throw std::runtime_error("Generate SVG failed");
    }
    return svg;
}

/*
// https://github.com/mapnik/mapnik/issues/1437
std::string to_svg2( path_type const& geom)
{
    std::string svg; // Use Python String directly ?
    bool result = mapnik::util::to_svg(svg,geom);
    if (!result)
    {
        throw std::runtime_error("Generate WKT failed");
    }
    return svg;
}*/


void export_geometry()
{
    using namespace boost::python;

    enum_<mapnik::geometry_type::types>("GeometryType")
        .value("Point",mapnik::geometry_type::types::Point)
        .value("LineString",mapnik::geometry_type::types::LineString)
        .value("Polygon",mapnik::geometry_type::types::Polygon)
        ;

    enum_<mapnik::util::wkbByteOrder>("wkbByteOrder")
        .value("XDR",mapnik::util::wkbXDR)
        .value("NDR",mapnik::util::wkbNDR)
        ;

    using mapnik::geometry_type;
    class_<geometry_type, std::shared_ptr<geometry_type>, boost::noncopyable>("Geometry2d",no_init)
        .def("envelope",&geometry_type::envelope)
        // .def("__str__",&geometry_type::to_string)
        .def("type",&geometry_type::type)
        .def("to_wkb",&to_wkb)
        .def("to_wkt",&to_wkt)
        .def("to_svg",&to_svg)
        // TODO add other geometry_type methods
        ;

    class_<path_type, std::shared_ptr<path_type>, boost::noncopyable>("Path")
        .def("__getitem__", getitem_impl,return_value_policy<reference_existing_object>())
        .def("__len__", &path_type::size)
        .def("envelope",envelope_impl)
        .def("add_wkt",add_wkt_impl)
        .def("add_wkb",add_wkb_impl)
        .def("add_geojson",add_geojson_impl)
        .def("to_wkt",&to_wkt2)
        //.def("to_svg",&to_svg2)
        .def("to_wkb",&to_wkb2)
        .def("from_wkt",from_wkt_impl)
        .def("from_wkb",from_wkb_impl)
        .def("from_geojson",from_geojson_impl)
        .def("to_geojson",to_geojson)
        .staticmethod("from_wkt")
        .staticmethod("from_wkb")
        .staticmethod("from_geojson")
        ;

}
