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
//$Id$

#include <boost/python.hpp>
#include <mapnik/raster_symbolizer.hpp>

using mapnik::raster_symbolizer;

struct raster_symbolizer_pickle_suite : boost::python::pickle_suite
{
   /*
   static boost::python::tuple
   getinitargs(const raster_symbolizer& r)
   {
      return boost::python::make_tuple();  
   }
   */

   static  boost::python::tuple
   getstate(const raster_symbolizer& r)
   {
        return boost::python::make_tuple(r.get_mode(),r.get_scaling(),r.get_opacity());
   }

   static void
   setstate (raster_symbolizer& r, boost::python::tuple state)
   {
        using namespace boost::python;
        if (len(state) != 3)
        {
            PyErr_SetObject(PyExc_ValueError,
                         ("expected 3-item tuple in call to __setstate__; got %s"
                          % state).ptr()
            );
            throw_error_already_set();
        }
                
        r.set_mode(extract<std::string>(state[0]));
        r.set_scaling(extract<std::string>(state[1]));
        r.set_opacity(extract<float>(state[2]));
   }

};

void export_raster_symbolizer()
{
    using namespace boost::python;

    class_<raster_symbolizer>("RasterSymbolizer",
				    init<>("Default ctor"))
				    
    .def_pickle(raster_symbolizer_pickle_suite())
    
    .add_property("mode",
            make_function(&raster_symbolizer::get_mode,return_value_policy<copy_const_reference>()),
            &raster_symbolizer::set_mode,
            "Get/Set merging mode.\n"
            "Possible values are:\n"
            "normal, grain_merge, grain_merge2, multiply,\n"
            "multiply2, divide, divide2, screen, and hard_light\n"
            "\n"
            "Usage:\n"
            "\n"
            ">>> from mapnik import RasterSymbolizer\n"
            ">>> r = RasterSymbolizer()\n"
            ">>> r.mode = 'grain_merge2'\n"
            )
            
    .add_property("scaling",
            make_function(&raster_symbolizer::get_scaling,return_value_policy<copy_const_reference>()),
            &raster_symbolizer::set_scaling,
            "Get/Set scaling algorithm.\n"
            "Possible values are:\n"
            "fast, bilinear, and bilinear8\n"
            "\n"
            "Usage:\n"
            "\n"
            ">>> from mapnik import RasterSymbolizer\n"
            ">>> r = RasterSymbolizer()\n"
            ">>> r.scaling = 'bilinear8'\n"
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
	;    
}
