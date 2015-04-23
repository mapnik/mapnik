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
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_type.hpp>
#include <mapnik/geometry_envelope.hpp>
#include <mapnik/geometry_is_valid.hpp>
#include <mapnik/geometry_is_simple.hpp>
#include <mapnik/geometry_is_empty.hpp>
#include <mapnik/geometry_correct.hpp>
#include <mapnik/geometry_centroid.hpp>

#include <mapnik/wkt/wkt_factory.hpp> // from_wkt
#include <mapnik/json/geometry_parser.hpp> // from_geojson
#include <mapnik/util/geometry_to_geojson.hpp> // to_geojson
#include <mapnik/util/geometry_to_wkb.hpp> // to_wkb
#include <mapnik/util/geometry_to_wkt.hpp> // to_wkt
//#include <mapnik/util/geometry_to_svg.hpp>
#include <mapnik/wkb.hpp>


// stl
#include <stdexcept>

namespace {

std::shared_ptr<mapnik::geometry::geometry<double> > from_wkb_impl(std::string const& wkb)
{
    std::shared_ptr<mapnik::geometry::geometry<double> > geom = std::make_shared<mapnik::geometry::geometry<double> >();
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

std::shared_ptr<mapnik::geometry::geometry<double> > from_wkt_impl(std::string const& wkt)
{
    std::shared_ptr<mapnik::geometry::geometry<double> > geom = std::make_shared<mapnik::geometry::geometry<double> >();
    if (!mapnik::from_wkt(wkt, *geom))
        throw std::runtime_error("Failed to parse WKT geometry");
    return geom;
}

std::shared_ptr<mapnik::geometry::geometry<double> > from_geojson_impl(std::string const& json)
{
    std::shared_ptr<mapnik::geometry::geometry<double> > geom = std::make_shared<mapnik::geometry::geometry<double> >();
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

PyObject* to_wkb_impl(mapnik::geometry::geometry<double> const& geom, mapnik::wkbByteOrder byte_order)
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

std::string to_geojson_impl(mapnik::geometry::geometry<double> const& geom)
{
    std::string wkt;
    if (!mapnik::util::to_geojson(wkt, geom))
    {
        throw std::runtime_error("Generate JSON failed");
    }
    return wkt;
}

std::string to_wkt_impl(mapnik::geometry::geometry<double> const& geom)
{
    std::string wkt;
    if (!mapnik::util::to_wkt(wkt,geom))
    {
        throw std::runtime_error("Generate WKT failed");
    }
    return wkt;
}

mapnik::geometry::geometry_types geometry_type_impl(mapnik::geometry::geometry<double> const& geom)
{
    return mapnik::geometry::geometry_type(geom);
}

mapnik::box2d<double> geometry_envelope_impl(mapnik::geometry::geometry<double> const& geom)
{
    return mapnik::geometry::envelope(geom);
}

// only Boost >= 1.56 has is_valid and is_simple
#if BOOST_VERSION >= 105600

bool geometry_is_valid_impl(mapnik::geometry::geometry<double> const& geom)
{
    return mapnik::geometry::is_valid(geom);
}

bool geometry_is_simple_impl(mapnik::geometry::geometry<double> const& geom)
{
    return mapnik::geometry::is_simple(geom);
}

#endif // BOOST_VERSION >= 1.56

bool geometry_is_empty_impl(mapnik::geometry::geometry<double> const& geom)
{
    return mapnik::geometry::is_empty(geom);
}

void geometry_correct_impl(mapnik::geometry::geometry<double> & geom)
{
    mapnik::geometry::correct(geom);
}

void polygon_set_exterior_impl(mapnik::geometry::polygon<double> & poly, mapnik::geometry::linear_ring<double> const& ring)
{
    poly.exterior_ring = ring; // copy
}

void polygon_add_hole_impl(mapnik::geometry::polygon<double> & poly, mapnik::geometry::linear_ring<double> const& ring)
{
    poly.interior_rings.push_back(ring); // copy
}

mapnik::geometry::point<double> geometry_centroid_impl(mapnik::geometry::geometry<double> const& geom)
{
    mapnik::geometry::point<double> pt;
    mapnik::geometry::centroid(geom, pt);
    return pt;
}


void export_geometry()
{
    using namespace boost::python;

    implicitly_convertible<mapnik::geometry::point<double>, mapnik::geometry::geometry<double> >();
    implicitly_convertible<mapnik::geometry::line_string<double>, mapnik::geometry::geometry<double> >();
    implicitly_convertible<mapnik::geometry::polygon<double>, mapnik::geometry::geometry<double> >();
    enum_<mapnik::geometry::geometry_types>("GeometryType")
        .value("Unknown",mapnik::geometry::geometry_types::Unknown)
        .value("Point",mapnik::geometry::geometry_types::Point)
        .value("LineString",mapnik::geometry::geometry_types::LineString)
        .value("Polygon",mapnik::geometry::geometry_types::Polygon)
        .value("MultiPoint",mapnik::geometry::geometry_types::MultiPoint)
        .value("MultiLineString",mapnik::geometry::geometry_types::MultiLineString)
        .value("MultiPolygon",mapnik::geometry::geometry_types::MultiPolygon)
        .value("GeometryCollection",mapnik::geometry::geometry_types::GeometryCollection)
        ;

    enum_<mapnik::wkbByteOrder>("wkbByteOrder")
        .value("XDR",mapnik::wkbXDR)
        .value("NDR",mapnik::wkbNDR)
        ;

    using mapnik::geometry::geometry;
    using mapnik::geometry::point;
    using mapnik::geometry::line_string;
    using mapnik::geometry::linear_ring;
    using mapnik::geometry::polygon;

    class_<point<double> >("Point", init<double, double>((arg("x"), arg("y")),
                                                "Constructs a new Point object\n"))
        .add_property("x", &point<double>::x, "X coordinate")
        .add_property("y", &point<double>::y, "Y coordinate")
// only Boost >= 1.56 has is_valid and is_simple
#if BOOST_VERSION >= 105600
        .def("is_valid", &geometry_is_valid_impl)
        .def("is_simple", &geometry_is_simple_impl)
#endif // BOOST_VERSION >= 1.56
        .def("to_geojson",&to_geojson_impl)
        .def("to_wkb",&to_wkb_impl)
        .def("to_wkt",&to_wkt_impl)
        ;

    class_<line_string<double> >("LineString", init<>(
                      "Constructs a new LineString object\n"))
        .def("add_coord", &line_string<double>::add_coord, "Adds coord")
// only Boost >= 1.56 has is_valid and is_simple
#if BOOST_VERSION >= 105600
        .def("is_valid", &geometry_is_valid_impl)
        .def("is_simple", &geometry_is_simple_impl)
#endif // BOOST_VERSION >= 1.56
        .def("to_geojson",&to_geojson_impl)
        .def("to_wkb",&to_wkb_impl)
        .def("to_wkt",&to_wkt_impl)
        ;

    class_<linear_ring<double> >("LinearRing", init<>(
                            "Constructs a new LinearRtring object\n"))
        .def("add_coord", &linear_ring<double>::add_coord, "Adds coord")
        ;

    class_<polygon<double> >("Polygon", init<>(
                        "Constructs a new Polygon object\n"))
        .add_property("exterior_ring", &polygon<double>::exterior_ring , "Exterior ring")
        .def("add_hole", &polygon_add_hole_impl, "Add interior ring")
        .def("num_rings", polygon_set_exterior_impl, "Number of rings (at least 1)")
// only Boost >= 1.56 has is_valid and is_simple
#if BOOST_VERSION >= 105600
        .def("is_valid", &geometry_is_valid_impl)
        .def("is_simple", &geometry_is_simple_impl)
#endif // BOOST_VERSION >= 1.56
        .def("to_geojson",&to_geojson_impl)
        .def("to_wkb",&to_wkb_impl)
        .def("to_wkt",&to_wkt_impl)
        ;

    class_<geometry<double>, std::shared_ptr<geometry<double> >, boost::noncopyable>("Geometry",no_init)
        .def("envelope",&geometry_envelope_impl)
        .def("from_geojson", from_geojson_impl)
        .def("from_wkt", from_wkt_impl)
        .def("from_wkb", from_wkb_impl)
        .staticmethod("from_geojson")
        .staticmethod("from_wkt")
        .staticmethod("from_wkb")
        .def("__str__",&to_wkt_impl)
        .def("type",&geometry_type_impl)
// only Boost >= 1.56 has is_valid and is_simple
#if BOOST_VERSION >= 105600
        .def("is_valid", &geometry_is_valid_impl)
        .def("is_simple", &geometry_is_simple_impl)
#endif // BOOST_VERSION >= 1.56
        .def("is_empty", &geometry_is_empty_impl)
        .def("correct", &geometry_correct_impl)
        .def("centroid",&geometry_centroid_impl)
        .def("to_wkb",&to_wkb_impl)
        .def("to_wkt",&to_wkt_impl)
        .def("to_geojson",&to_geojson_impl)
        //.def("to_svg",&to_svg)
        // TODO add other geometry_type methods
        ;
}
