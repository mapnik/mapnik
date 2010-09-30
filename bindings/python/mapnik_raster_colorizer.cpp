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
//$Id$

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <mapnik/raster_colorizer.hpp>

using mapnik::raster_colorizer;
using mapnik::raster_colorizer_ptr;
using mapnik::color_band;
using mapnik::color_bands;
using mapnik::color;

namespace {
void append_band1(raster_colorizer_ptr & rc, color_band b)
{
    rc->append_band(b);
}
void append_band2(raster_colorizer_ptr & rc, color_band b, unsigned m)
{
    rc->append_band(b, m);
}
void append_band3(raster_colorizer_ptr & rc, float v, color c)
{
    rc->append_band(v, c);
}
void append_band4(raster_colorizer_ptr & rc, float v, color c, unsigned m)
{
    rc->append_band(v, c, m);
}
void append_band5(raster_colorizer_ptr & rc, float v, float vm, color c, unsigned m)
{
    rc->append_band(v, vm, c, m);
}
void append_band6(raster_colorizer_ptr & rc, float v, float vm, color c)
{
    rc->append_band(v, vm, c);
}
color_bands const& get_color_bands(raster_colorizer_ptr & rc)
{
    return rc->get_color_bands();
}
}

void export_raster_colorizer()
{
    using namespace boost::python;

    class_<raster_colorizer,raster_colorizer_ptr>("RasterColorizer", init<>("Default ctor."))
                                    
        .add_property("bands",make_function
                      (get_color_bands,
                       return_value_policy<reference_existing_object>()))
        .def("append_band", append_band1,
            (arg("color_band")),
            "Append a color band to the raster colorizer.\n"
            "\n"
            "Usage:\n"
            ">>> colorizer = mapnik.ColorBand()\n"
            ">>> color = mapnik.Color(\"#0044cc\")\n"
            ">>> color_band = mapnik.ColorBand(3, color)\n"
            ">>> colorizer.append_band(color_band)\n"
            )
        .def("append_band", append_band2,
            (arg("color_band"), arg("midpoints")),
            "Append a color band to the raster colorizer with midpoints "
            "lineally interpolated color bands between this one and the "
            "previous one.\n"
            "\n"
            "Usage:\n"
            ">>> colorizer = mapnik.ColorBand()\n"
            ">>> color = mapnik.Color(\"#0044cc\")\n"
            ">>> color_band = mapnik.ColorBand(3, color)\n"
            ">>> colorizer.append_band(color_band, 1)\n"
            )
        .def("append_band", append_band3, 
            (arg("value"), arg("color")),
            "Create and append a color band to color the range "
            "[value, next_val) where next_val is the next band's color or "
            "inifinity if there is no next band.\n"
            "Usage:\n"
            ">>> colorizer = mapnik.RasterColorizer()\n"
            ">>> color = mapnik.Color(\"#0044cc\")\n"
            ">>> colorizer.append_band(30, color)\n"
            )
        .def("append_band", append_band4, 
            (arg("value"), arg("color"), arg("midpoints")),
            "Create and append a color band to the raster colorizer with "
            "midpoints lineally interpolated color bands between this one and "
            "the previous one.\n"
            "color will be applied to all values in the "
            "range [value, next_val) where next_val is the next band's color "
            "or infinity if there is no next band\n"
            "\n"
            "Usage:\n"
            ">>> colorizer = mapnik.RasterColorizer()\n"
            ">>> color = mapnik.Color(\"#0044cc\")\n"
            ">>> colorizer.append_band(30, color, 4)\n"
            )
        .def("append_band", append_band5, 
            (arg("value"), arg("value_max"), arg("color"), arg("midpoints")),
            "Create and append a color band to the raster colorizer with "
            "midpoints lineally interpolated color bands between this one and "
            "the previous one.\n"
            "color will be applied to all values in the "
            "range [value, next_val) where next_val is the next band's color "
            "or value_max if there is no next band\n"
            "\n"
            "\n"
            "Usage:\n"
            ">>> colorizer = mapnik.RasterColorizer()\n"
            ">>> color = mapnik.Color(\"#0044cc\")\n"
            ">>> colorizer.append_band(30, 40, color, 4)\n"
            )
        .def("append_band", append_band6, 
            (arg("value"), arg("value_max"), arg("color")),
            "Create and append a color band to color the range "
            "[value, next_val) where next_val is the next band's color or "
            "value_max if there is no next band.\n"
            "Usage:\n"
            ">>> colorizer = mapnik.RasterColorizer()\n"
            ">>> color = mapnik.Color(\"#0044cc\")\n"
            ">>> colorizer.append_band(30, 40, color)\n"
            )
        .def("get_color", &raster_colorizer::get_color, 
            "Get the color assigned to a certain value in raster data.\n"
            "By default, returns Color(\"transparent\")\n"
            "\n"
            "Usage:\n"
            ">>> colorizer = mapnik.RasterColorizer()\n"
            ">>> color = mapnik.Color(\"#0044cc\")\n"
            ">>> colorizer.append_band(30, 40, color)\n"
            ">>> colorizer.get_color(35)\n"
            "Color('#0044cc')\n"
            )
        ;    



    class_<color_bands>("ColorBands",
        "A RasterColorizer's collection of ordered color bands.\n"
        "This class is not meant to be instantiated from python. However, "
        "it can be accessed at a RasterColorizer's \"bands\" attribute for "
        "introspection purposes",
        no_init)
        .def(vector_indexing_suite<color_bands>())
        ;


    class_<color_band>("ColorBand",init<float,color const&>(
                       "A Color Band object.\n"
                       "Create with a value and color\n"
                       "\n"
                       "Usage:"
                       ">>> color = mapnik.Color(\"#fff000\")\n"
                       ">>> color_band = mapnik.ColorBand(4, color)\n"
          ))
        .add_property("color", make_function
                      (&color_band::get_color,
                       return_value_policy<reference_existing_object>()))
        .add_property("value", &color_band::get_value)
        .add_property("max_value", &color_band::get_max_value)
        .def(self == self)
        .def("__str__",&color_band::to_string)
        ;
}
