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
   color_bands const& get_color_bands(raster_colorizer_ptr & rc)
   {
      return rc->get_color_bands();
   }
}

void export_raster_colorizer()
{
    using namespace boost::python;

    class_<raster_colorizer,raster_colorizer_ptr>("RasterColorizer", init<>("Deafult ctor."))
				    
    .add_property("bands",make_function
                  (get_color_bands,
                  return_value_policy<reference_existing_object>()))
    .def("append_band", append_band1, "TODO: Write docs")
    .def("append_band", append_band2, "TODO: Write docs")
    .def("append_band", append_band3, "TODO: Write docs")
    .def("append_band", append_band4, "TODO: Write docs")
    .def("get_color", &raster_colorizer::get_color, "TODO: Write docs")
	;    



    class_<color_bands>("ColorBands",init<>("Default ctor."))
    	.def(vector_indexing_suite<color_bands>())
    	;


    class_<color_band>("ColorBand",
                       init<float,color const&>("Deafult ctor."))
    .add_property("color", make_function
                  (&color_band::get_color,
                   return_value_policy<reference_existing_object>()))
    .add_property("value", &color_band::get_value)
    .def(self == self)
    .def("__str__",&color_band::to_string)
    ;
}
