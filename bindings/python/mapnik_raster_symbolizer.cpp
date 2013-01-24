/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko, Jean-Francois Doyon
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

// boost
#include <boost/python.hpp>

// mapnik
#include <mapnik/raster_symbolizer.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/image_scaling.hpp>

namespace {

// https://github.com/mapnik/mapnik/issues/1367
PyObject* get_premultiplied_impl(mapnik::raster_symbolizer & sym)
{
    boost::optional<bool> premultiplied = sym.premultiplied();
    if (premultiplied)
        return ::PyBool_FromLong(*premultiplied);
    Py_RETURN_NONE;
}

}
using mapnik::raster_symbolizer;

void export_raster_symbolizer()
{
    using namespace boost::python;

    class_<raster_symbolizer>("RasterSymbolizer",
                              init<>("Default ctor"))

        .add_property("mode",
                      make_function(&raster_symbolizer::get_mode,return_value_policy<copy_const_reference>()),
                      &raster_symbolizer::set_mode,
                      "Get/Set merging mode. (deprecated, use comp_op instead)\n"
            )
        .add_property("comp_op",
                      &raster_symbolizer::comp_op,
                      &raster_symbolizer::set_comp_op,
                      "Set/get the raster comp-op"
            )
        .add_property("scaling",
                      &raster_symbolizer::get_scaling_method,
                      &raster_symbolizer::set_scaling_method,
                      "Get/Set scaling algorithm.\n"
                      "\n"
                      "Usage:\n"
                      "\n"
                      ">>> from mapnik import RasterSymbolizer\n"
                      ">>> r = RasterSymbolizer()\n"
                      ">>> r.scaling = 'mapnik.scaling_method.GAUSSIAN'\n"
            )

        .add_property("opacity",
                      &raster_symbolizer::get_opacity,
                      &raster_symbolizer::set_opacity,
                      "Get/Set opacity.\n"
                      "\n"
                      "Usage:\n"
                      "\n"
                      ">>> from mapnik import RasterSymbolizer\n"
                      ">>> r = RasterSymbolizer()\n"
                      ">>> r.opacity = .5\n"
            )
        .add_property("colorizer",
                      &raster_symbolizer::get_colorizer,
                      &raster_symbolizer::set_colorizer,
                      "Get/Set the RasterColorizer used to color data rasters.\n"
                      "\n"
                      "Usage:\n"
                      "\n"
                      ">>> from mapnik import RasterSymbolizer, RasterColorizer\n"
                      ">>> r = RasterSymbolizer()\n"
                      ">>> r.colorizer = RasterColorizer()\n"
                      ">>> for value, color in [\n"
                      "...     (0, \"#000000\"),\n"
                      "...     (10, \"#ff0000\"),\n"
                      "...     (40, \"#00ff00\"),\n"
                      "... ]:\n"
                      "...      r.colorizer.append_band(value, color)\n"
            )
        .add_property("filter_factor",
                      &raster_symbolizer::get_filter_factor,
                      &raster_symbolizer::set_filter_factor,
                      "Get/Set the filter factor used by the datasource.\n"
                      "\n"
                      "This is used by the Raster or Gdal datasources to pre-downscale\n"
                      "images using overviews.\n"
                      "Higher numbers can sometimes cause much better scaled image\n"
                      "output, at the cost of speed.\n"
                      "\n"
                      "Examples:\n"
                      " -1.0 : (Default) A suitable value will be determined from the\n"
                      "        chosen scaling method during rendering.\n"
                      "  1.0 : The datasource will take care of all the scaling\n"
                      "        (using nearest neighbor interpolation)\n"
                      "  2.0 : The datasource will scale the datasource to\n"
                      "        2.0x the desired size, and mapnik will scale the rest\n"
                      "        of the way using the interpolation defined in self.scaling.\n"
            )
        .add_property("mesh_size",
                      &raster_symbolizer::get_mesh_size,
                      &raster_symbolizer::set_mesh_size,
                      "Get/Set warping mesh size.\n"
                      "Larger values result in faster warping times but might "
                      "result in distorted maps.\n"
                      "\n"
                      "Usage:\n"
                      "\n"
                      ">>> from mapnik import RasterSymbolizer\n"
                      ">>> r = RasterSymbolizer()\n"
                      ">>> r.mesh_size = 32\n"
            )
        .add_property("premultiplied",
                      &get_premultiplied_impl,
                      &raster_symbolizer::set_premultiplied,
                      "Get/Set premultiplied status of the source image.\n"
                      "Can be used to override what the source data reports (when in error)\n"
                      "\n"
                      "Usage:\n"
                      "\n"
                      ">>> from mapnik import RasterSymbolizer\n"
                      ">>> r = RasterSymbolizer()\n"
                      ">>> r.premultiplied = False\n"
            )
        ;
}
