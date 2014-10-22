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

#include <mapnik/config.hpp>

// boost
#include "boost_std_shared_shim.hpp"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/exception_translator.hpp>
#include <boost/python/manage_new_object.hpp>
#include <boost/python/iterator.hpp>
#include <boost/noncopyable.hpp>
#include <boost/version.hpp>
#pragma GCC diagnostic pop

// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_container.hpp>
#include <mapnik/wkt/wkt_factory.hpp> // from_wkt
#include <mapnik/util/geometry_to_wkt.hpp>
#include <mapnik/json/geometry_parser.hpp> // from_geojson
#include <mapnik/util/geometry_to_geojson.hpp>
#include <mapnik/util/geometry_to_svg.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/util/geometry_to_wkb.hpp>

// stl
#include <stdexcept>

namespace {

mapnik::geometry_type const& getitem_impl(mapnik::geometry_container & p, int key)
{
    if (key >=0 && key < static_cast<int>(p.size()))
        return p[key];
    PyErr_SetString(PyExc_IndexError, "Index is out of range");
    throw boost::python::error_already_set();
}

void add_wkt_impl(mapnik::geometry_container& p, std::string const& wkt)
{
    if (!mapnik::from_wkt(wkt , p))
        throw std::runtime_error("Failed to parse WKT");
}

void add_wkb_impl(mapnik::geometry_container& p, std::string const& wkb)
{
    if (!mapnik::geometry_utils::from_wkb(p, wkb.c_str(), wkb.size()))
        throw std::runtime_error("Failed to parse WKB");
}

void add_geojson_impl(mapnik::geometry_container& paths, std::string const& json)
{
    if (!mapnik::json::from_geojson(json, paths))
        throw std::runtime_error("Failed to parse geojson geometry");
}

std::shared_ptr<mapnik::geometry_container> from_wkt_impl(std::string const& wkt)
{
    std::shared_ptr<mapnik::geometry_container> paths = std::make_shared<mapnik::geometry_container>();
    if (!mapnik::from_wkt(wkt, *paths))
        throw std::runtime_error("Failed to parse WKT");
    return paths;
}

std::shared_ptr<mapnik::geometry_container> from_wkb_impl(std::string const& wkb)
{
    std::shared_ptr<mapnik::geometry_container> paths = std::make_shared<mapnik::geometry_container>();
    if (!mapnik::geometry_utils::from_wkb(*paths, wkb.c_str(), wkb.size()))
        throw std::runtime_error("Failed to parse WKB");
    return paths;
}

std::shared_ptr<mapnik::geometry_container> from_geojson_impl(std::string const& json)
{
    std::shared_ptr<mapnik::geometry_container> paths = std::make_shared<mapnik::geometry_container>();
    if (!mapnik::json::from_geojson(json, *paths))
        throw std::runtime_error("Failed to parse geojson geometry");
    return paths;
}

mapnik::box2d<double> envelope_impl(mapnik::geometry_container & p)
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

PyObject* to_wkb(mapnik::geometry_type const& geom, mapnik::util::wkbByteOrder byte_order)
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

PyObject* to_wkb2( mapnik::geometry_container const& p, mapnik::util::wkbByteOrder byte_order)
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

std::string to_wkt(mapnik::geometry_type const& geom)
{
    std::string wkt;
    if (!mapnik::util::to_wkt(wkt,geom))
    {
        throw std::runtime_error("Generate WKT failed");
    }
    return wkt;
}

std::string to_wkt2(mapnik::geometry_container const& geom)
{
    std::string wkt;
    if (!mapnik::util::to_wkt(wkt,geom))
    {
        throw std::runtime_error("Generate WKT failed");
    }
    return wkt;
}

std::string to_geojson(mapnik::geometry_type const& geom)
{
    std::string wkt;
    if (!mapnik::util::to_geojson(wkt,geom))
    {
        throw std::runtime_error("Generate JSON failed");
    }
    return wkt;
}

std::string to_geojson2(mapnik::geometry_container const& geom)
{
    std::string wkt;
    if (!mapnik::util::to_geojson(wkt,geom))
    {
        throw std::runtime_error("Generate JSON failed");
    }
    return wkt;
}

std::string to_svg(mapnik::geometry_type const& geom)
{
    std::string svg;
    if (!mapnik::util::to_svg(svg,geom))
    {
        throw std::runtime_error("Generate SVG failed");
    }
    return svg;
}

/*
// https://github.com/mapnik/mapnik/issues/1437
std::string to_svg2( mapnik::geometry_container const& geom)
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
    class_<mapnik::geometry_type, std::shared_ptr<mapnik::geometry_type>, boost::noncopyable>("Geometry2d",no_init)
        .def("envelope",&mapnik::geometry_type::envelope)
        // .def("__str__",&mapnik::geometry_type::to_string)
        .def("type",&mapnik::geometry_type::type)
        .def("to_wkb",&to_wkb)
        .def("to_wkt",&to_wkt)
        .def("to_geojson",&to_geojson)
        .def("to_svg",&to_svg)
        // TODO add other geometry_type methods
        ;

    class_<mapnik::geometry_container, std::shared_ptr<mapnik::geometry_container>, boost::noncopyable>("Path")
        .def("__getitem__", getitem_impl,return_value_policy<reference_existing_object>())
        .def("__len__", &mapnik::geometry_container::size)
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
        .def("to_geojson",&to_geojson2)
        .staticmethod("from_wkt")
        .staticmethod("from_wkb")
        .staticmethod("from_geojson")
        ;

}
