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
#include <mapnik/image_util.hpp>
#include <mapnik/polygon_pattern_symbolizer.hpp>

using mapnik::polygon_pattern_symbolizer;

struct polygon_pattern_symbolizer_pickle_suite : boost::python::pickle_suite
{
   static boost::python::tuple
   getinitargs(const polygon_pattern_symbolizer& p)
   {
      boost::shared_ptr<mapnik::ImageData32> img = p.get_image();
      const std::string & filename = p.get_filename();
      return boost::python::make_tuple(filename,mapnik::guess_type(filename),img->width(),img->height());
   }
};

void export_polygon_pattern_symbolizer()
{
    using namespace boost::python;
    
    class_<polygon_pattern_symbolizer>("PolygonPatternSymbolizer",
				       init<std::string const&,
				       std::string const&,
				       unsigned,unsigned>("TODO"))
        .def_pickle(polygon_pattern_symbolizer_pickle_suite())	;    
}
