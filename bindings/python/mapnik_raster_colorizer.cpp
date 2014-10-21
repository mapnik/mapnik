/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Artem Pavlenko
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
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#pragma GCC diagnostic pop

// mapnik
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/symbolizer.hpp>

using mapnik::raster_colorizer;
using mapnik::raster_colorizer_ptr;
using mapnik::symbolizer_base;
using mapnik::colorizer_stop;
using mapnik::colorizer_stops;
using mapnik::colorizer_mode_enum;
using mapnik::color;
using mapnik::COLORIZER_INHERIT;
using mapnik::COLORIZER_LINEAR;
using mapnik::COLORIZER_DISCRETE;
using mapnik::COLORIZER_EXACT;


namespace {
void add_stop(raster_colorizer_ptr & rc, colorizer_stop & stop)
{
    rc->add_stop(stop);
}
void add_stop2(raster_colorizer_ptr & rc, float v) {
    colorizer_stop stop(v, rc->get_default_mode(), rc->get_default_color());
    rc->add_stop(stop);
}
void add_stop3(raster_colorizer_ptr &rc, float v, color c) {
    colorizer_stop stop(v, rc->get_default_mode(), c);
    rc->add_stop(stop);
}
void add_stop4(raster_colorizer_ptr &rc, float v, colorizer_mode_enum m) {
    colorizer_stop stop(v, m, rc->get_default_color());
    rc->add_stop(stop);
}
void add_stop5(raster_colorizer_ptr &rc, float v, colorizer_mode_enum m, color c) {
    colorizer_stop stop(v, m, c);
    rc->add_stop(stop);
}
mapnik::color get_color(raster_colorizer_ptr &rc, float value) {
    unsigned rgba = rc->get_color(value);
    unsigned r = (rgba & 0xff);
    unsigned g = (rgba >> 8 ) & 0xff;
    unsigned b = (rgba >> 16) & 0xff;
    unsigned a = (rgba >> 24) & 0xff;
    return mapnik::color(r,g,b,a);
}

colorizer_stops const& get_stops(raster_colorizer_ptr & rc)
{
    return rc->get_stops();
}
}

void export_raster_colorizer()
{
    using namespace boost::python;

    implicitly_convertible<raster_colorizer_ptr, mapnik::symbolizer_base::value_type>();

    class_<raster_colorizer,raster_colorizer_ptr>("RasterColorizer",
                                                  "A Raster Colorizer object.",
                                                  init<colorizer_mode_enum, color>(args("default_mode","default_color"))
        )
        .def(init<>())
        .add_property("default_color",
                      make_function(&raster_colorizer::get_default_color, return_value_policy<reference_existing_object>()),
                      &raster_colorizer::set_default_color,
                      "The default color for stops added without a color (mapnik.Color).\n")
        .add_property("default_mode",
                      &raster_colorizer::get_default_mode_enum,
                      &raster_colorizer::set_default_mode_enum,
                      "The default mode (mapnik.ColorizerMode).\n"
                      "\n"
                      "If a stop is added without a mode, then it will inherit this default mode\n")
        .add_property("stops",
                      make_function(get_stops,return_value_policy<reference_existing_object>()),
                      "The list of stops this RasterColorizer contains\n")
        .add_property("epsilon",
                      &raster_colorizer::get_epsilon,
                      &raster_colorizer::set_epsilon,
                      "Comparison epsilon value for exact mode\n"
                      "\n"
                      "When comparing values in exact mode, values need only be within epsilon to match.\n")


        .def("add_stop", add_stop,
             (arg("ColorizerStop")),
             "Add a colorizer stop to the raster colorizer.\n"
             "\n"
             "Usage:\n"
             ">>> colorizer = mapnik.RasterColorizer()\n"
             ">>> color = mapnik.Color(\"#0044cc\")\n"
             ">>> stop = mapnik.ColorizerStop(3, mapnik.COLORIZER_INHERIT, color)\n"
             ">>> colorizer.add_stop(stop)\n"
            )
        .def("add_stop", add_stop2,
             (arg("value")),
             "Add a colorizer stop to the raster colorizer, using the default mode and color.\n"
             "\n"
             "Usage:\n"
             ">>> default_color = mapnik.Color(\"#0044cc\")\n"
             ">>> colorizer = mapnik.RasterColorizer(mapnik.COLORIZER_LINEAR, default_color)\n"
             ">>> colorizer.add_stop(100)\n"
            )
        .def("add_stop", add_stop3,
             (arg("value")),
             "Add a colorizer stop to the raster colorizer, using the default mode.\n"
             "\n"
             "Usage:\n"
             ">>> default_color = mapnik.Color(\"#0044cc\")\n"
             ">>> colorizer = mapnik.RasterColorizer(mapnik.COLORIZER_LINEAR, default_color)\n"
             ">>> colorizer.add_stop(100, mapnik.Color(\"#123456\"))\n"
            )
        .def("add_stop", add_stop4,
             (arg("value")),
             "Add a colorizer stop to the raster colorizer, using the default color.\n"
             "\n"
             "Usage:\n"
             ">>> default_color = mapnik.Color(\"#0044cc\")\n"
             ">>> colorizer = mapnik.RasterColorizer(mapnik.COLORIZER_LINEAR, default_color)\n"
             ">>> colorizer.add_stop(100, mapnik.COLORIZER_EXACT)\n"
            )
        .def("add_stop", add_stop5,
             (arg("value")),
             "Add a colorizer stop to the raster colorizer.\n"
             "\n"
             "Usage:\n"
             ">>> default_color = mapnik.Color(\"#0044cc\")\n"
             ">>> colorizer = mapnik.RasterColorizer(mapnik.COLORIZER_LINEAR, default_color)\n"
             ">>> colorizer.add_stop(100, mapnik.COLORIZER_DISCRETE, mapnik.Color(\"#112233\"))\n"
            )
        .def("get_color", get_color,
             "Get the color assigned to a certain value in raster data.\n"
             "\n"
             "Usage:\n"
             ">>> colorizer = mapnik.RasterColorizer()\n"
             ">>> color = mapnik.Color(\"#0044cc\")\n"
             ">>> colorizer.add_stop(0, mapnik.COLORIZER_DISCRETE, mapnik.Color(\"#000000\"))\n"
             ">>> colorizer.add_stop(100, mapnik.COLORIZER_DISCRETE, mapnik.Color(\"#0E0A06\"))\n"
             ">>> colorizer.get_color(50)\n"
             "Color('#070503')\n"
            )
        ;



    class_<colorizer_stops>("ColorizerStops",
                            "A RasterColorizer's collection of ordered color stops.\n"
                            "This class is not meant to be instantiated from python. However, "
                            "it can be accessed at a RasterColorizer's \"stops\" attribute for "
                            "introspection purposes",
                            no_init)
        .def(vector_indexing_suite<colorizer_stops>())
        ;

    enum_<colorizer_mode_enum>("ColorizerMode")
        .value("COLORIZER_INHERIT", COLORIZER_INHERIT)
        .value("COLORIZER_LINEAR", COLORIZER_LINEAR)
        .value("COLORIZER_DISCRETE", COLORIZER_DISCRETE)
        .value("COLORIZER_EXACT", COLORIZER_EXACT)
        .export_values()
        ;


    class_<colorizer_stop>("ColorizerStop",init<float, colorizer_mode_enum, color const&>(
                               "A Colorizer Stop object.\n"
                               "Create with a value, ColorizerMode, and Color\n"
                               "\n"
                               "Usage:"
                               ">>> color = mapnik.Color(\"#fff000\")\n"
                               ">>> stop= mapnik.ColorizerStop(42.42, mapnik.COLORIZER_LINEAR, color)\n"
                               ))
        .add_property("color",
                      make_function(&colorizer_stop::get_color, return_value_policy<reference_existing_object>()),
                      &colorizer_stop::set_color,
                      "The stop color (mapnik.Color).\n")
        .add_property("value",
                      &colorizer_stop::get_value,
                      &colorizer_stop::set_value,
                      "The stop value.\n")
        .add_property("label",
                      make_function(&colorizer_stop::get_label, return_value_policy<copy_const_reference>()),
                      &colorizer_stop::set_label,
                      "The stop label.\n")
        .add_property("mode",
                      &colorizer_stop::get_mode_enum,
                      &colorizer_stop::set_mode_enum,
                      "The stop mode (mapnik.ColorizerMode).\n"
                      "\n"
                      "If this is COLORIZER_INHERIT then it will inherit the default mode\n"
                      " from the RasterColorizer it is added to.\n")
        .def(self == self)
        .def("__str__",&colorizer_stop::to_string)
        ;
}
