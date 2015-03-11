/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <mapnik/geometry_impl.hpp>
#include <mapnik/geometry_type.hpp>
#include <mapnik/geometry_envelope.hpp>
#include <mapnik/geometry_is_valid.hpp>
#include <mapnik/geometry_is_simple.hpp>
#include <mapnik/geometry_correct.hpp>

#include <mapnik/wkt/wkt_factory.hpp> // from_wkt
//#include <mapnik/util/geometry_to_wkt.hpp>
#include <mapnik/json/geometry_parser.hpp> // from_geojson
#include <mapnik/util/geometry_to_geojson.hpp>
//#include <mapnik/util/geometry_to_svg.hpp>
#include <mapnik/wkb.hpp>
//#include <mapnik/util/geometry_to_wkb.hpp>

// stl
#include <stdexcept>

namespace {

//mapnik::geometry_type const& getitem_impl(mapnik::geometry_container & p, int key)
//{
//    if (key >=0 && key < static_cast<int>(p.size()))
//        return p[key];
//    PyErr_SetString(PyExc_IndexError, "Index is out of range");
//    throw boost::python::error_already_set();
//}

//void add_wkt_impl(mapnik::geometry_container& p, std::string const& wkt)
//{
//    if (!mapnik::from_wkt(wkt , p))
//        throw std::runtime_error("Failed to parse WKT");
//}

//void add_wkb_impl(mapnik::geometry_container& p, std::string const& wkb)
//{
//    if (!mapnik::geometry_utils::from_wkb(p, wkb.c_str(), wkb.size()))
//        throw std::runtime_error("Failed to parse WKB");
//}

//void add_geojson_impl(mapnik::geometry_container& paths, std::string const& json)
//{
//    if (!mapnik::json::from_geojson(json, paths))
//        throw std::runtime_error("Failed to parse geojson geometry");
//}

//std::shared_ptr<mapnik::geometry_container> from_wkt_impl(std::string const& wkt)
//{
//    std::shared_ptr<mapnik::geometry_container> paths = std::make_shared<mapnik::geometry_container>();
//    if (!mapnik::from_wkt(wkt, *paths))
//        throw std::runtime_error("Failed to parse WKT");
//    return paths;
//}

std::shared_ptr<mapnik::new_geometry::geometry> from_wkb_impl(std::string const& wkb)
{
    std::shared_ptr<mapnik::new_geometry::geometry> geom = std::make_shared<mapnik::new_geometry::geometry>();
    try
    {
        *geom = std::move(mapnik::geometry_utils::from_wkb(wkb.c_str(), wkb.size()));
    }
    catch (...)
    {
        throw std::runtime_error("Failed to parse WKB");
    }
    return geom;
}

std::shared_ptr<mapnik::new_geometry::geometry> from_wkt_impl(std::string const& wkt)
{
    std::shared_ptr<mapnik::new_geometry::geometry> geom = std::make_shared<mapnik::new_geometry::geometry>();
    if (!mapnik::from_wkt(wkt, *geom))
        throw std::runtime_error("Failed to parse WKT geometry");
    return geom;
}

std::shared_ptr<mapnik::new_geometry::geometry> from_geojson_impl(std::string const& json)
{
    std::shared_ptr<mapnik::new_geometry::geometry> geom = std::make_shared<mapnik::new_geometry::geometry>();
    if (!mapnik::json::from_geojson(json, *geom))
        throw std::runtime_error("Failed to parse geojson geometry");
    return geom;
}

}

inline std::string boost_version()
{
    std::ostringstream s;
    s << BOOST_VERSION/100000 << "." << BOOST_VERSION/100 % 1000  << "." << BOOST_VERSION % 100;
    return s.str();
}

//PyObject* to_wkb(mapnik::geometry_type const& geom, mapnik::util::wkbByteOrder byte_order)
//{
//   mapnik::util::wkb_buffer_ptr wkb = mapnik::util::to_wkb(geom,byte_order);
//   if (wkb)
//   {
//       return
//#if PY_VERSION_HEX >= 0x03000000
//            ::PyBytes_FromStringAndSize
//#else
//            ::PyString_FromStringAndSize
//#endif
//            ((const char*)wkb->buffer(),wkb->size());
//    }
//    else
//    {
//        Py_RETURN_NONE;
//    }
//}

//PyObject* to_wkb2( mapnik::geometry_container const& p, mapnik::util::wkbByteOrder byte_order)
//{
//    mapnik::util::wkb_buffer_ptr wkb = mapnik::util::to_wkb(p,byte_order);
//    if (wkb)
//    {
//        return
//#if PY_VERSION_HEX >= 0x03000000
//            ::PyBytes_FromStringAndSize
//#else
//            ::PyString_FromStringAndSize
//#endif
//            ((const char*)wkb->buffer(),wkb->size());
//    }
//    else
//    {
//        Py_RETURN_NONE;
//    }
//}

//std::string to_wkt(mapnik::geometry_type const& geom)
//{
//    std::string wkt;
//    if (!mapnik::util::to_wkt(wkt,geom))
//    {
//        throw std::runtime_error("Generate WKT failed");
//    }
//    return wkt;
//}

//std::string to_wkt2(mapnik::geometry_container const& geom)
//{
//    std::string wkt;
//    if (!mapnik::util::to_wkt(wkt,geom))
//    {
//        throw std::runtime_error("Generate WKT failed");
//    }
//    return wkt;
//}

std::string to_geojson_impl(mapnik::new_geometry::geometry const& geom)
{
    std::string wkt;
    if (!mapnik::util::to_geojson(wkt, geom))
    {
        throw std::runtime_error("Generate JSON failed");
    }
    return wkt;
}

//std::string to_geojson2(mapnik::geometry_container const& geom)
//{
//    std::string wkt;
//    if (!mapnik::util::to_geojson(wkt,geom))
//    {
//        throw std::runtime_error("Generate JSON failed");
//    }
//    return wkt;
//}

//std::string to_svg(mapnik::geometry_type const& geom)
//{
//    std::string svg;
//    if (!mapnik::util::to_svg(svg, geom))
//    {
//        throw std::runtime_error("Generate SVG failed");
//    }
//    return svg;
//}

mapnik::new_geometry::geometry_types geometry_type_impl(mapnik::new_geometry::geometry const& geom)
{
    return mapnik::new_geometry::geometry_type(geom);
}

mapnik::box2d<double> geometry_envelope_impl(mapnik::new_geometry::geometry const& geom)
{
    return mapnik::new_geometry::envelope(geom);
}

bool geometry_is_valid_impl(mapnik::new_geometry::geometry const& geom)
{
    return mapnik::new_geometry::is_valid(geom);
}

bool geometry_is_simple_impl(mapnik::new_geometry::geometry const& geom)
{
    return mapnik::new_geometry::is_simple(geom);
}

void geometry_correct_impl(mapnik::new_geometry::geometry & geom)
{
    mapnik::new_geometry::correct(geom);
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

    enum_<mapnik::new_geometry::geometry_types>("GeometryType")
        .value("Point",mapnik::new_geometry::geometry_types::Point)
        .value("LineString",mapnik::new_geometry::geometry_types::LineString)
        .value("Polygon",mapnik::new_geometry::geometry_types::Polygon)
        .value("MultiPoint",mapnik::new_geometry::geometry_types::MultiPoint)
        .value("MultiLineString",mapnik::new_geometry::geometry_types::MultiLineString)
        .value("MultiPolygon",mapnik::new_geometry::geometry_types::MultiPolygon)
        .value("GeometryCollection",mapnik::new_geometry::geometry_types::GeometryCollection)
        ;

    enum_<mapnik::wkbByteOrder>("wkbByteOrder")
        .value("XDR",mapnik::wkbXDR)
        .value("NDR",mapnik::wkbNDR)
        ;

    using mapnik::new_geometry::geometry;

    class_<geometry, std::shared_ptr<geometry>, boost::noncopyable>("Geometry",no_init)
        .def("envelope",&geometry_envelope_impl)
        .def("from_geojson", from_geojson_impl)
        .def("from_wkt", from_wkt_impl)
        .def("from_wkb", from_wkb_impl)
        .staticmethod("from_geojson")
        .staticmethod("from_wkt")
        .staticmethod("from_wkb")
        // .def("__str__",&mapnik::geometry_type::to_string)
        .def("type",&geometry_type_impl)
        .def("is_valid", &geometry_is_valid_impl)
        .def("is_simple", &geometry_is_simple_impl)
        .def("correct", &geometry_correct_impl)
        //.def("to_wkb",&to_wkb)
        //.def("to_wkt",&to_wkt)
        .def("to_geojson",&to_geojson_impl)
        //.def("to_svg",&to_svg)
        // TODO add other geometry_type methods
        ;

    //class_<mapnik::geometry_container, std::shared_ptr<mapnik::geometry_container>, boost::noncopyable>("Path")
    //    .def("__getitem__", getitem_impl,return_value_policy<reference_existing_object>())
    //    .def("__len__", &mapnik::geometry_container::size)
    //    .def("envelope",envelope_impl2)
    //    .def("add_wkt",add_wkt_impl)
    //    .def("add_wkb",add_wkb_impl)
    //    .def("add_geojson",add_geojson_impl)
    //    .def("to_wkt",&to_wkt2)
        //.def("to_svg",&to_svg2)
    //    .def("to_wkb",&to_wkb2)
    //    .def("from_wkt",from_wkt_impl)
    //    .def("from_wkb",from_wkb_impl)
    //    .def("from_geojson",from_geojson_impl)
    //    .def("to_geojson",&to_geojson2)
    //    .staticmethod("from_wkt")
    //    .staticmethod("from_wkb")
    //   .staticmethod("from_geojson")
    //   ;

}
