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
#include <mapnik/path_expression_grammar.hpp>

using mapnik::polygon_pattern_symbolizer;
using mapnik::path_expression_ptr;
using mapnik::path_processor_type;
using mapnik::guess_type;

struct polygon_pattern_symbolizer_pickle_suite : boost::python::pickle_suite
{
   static boost::python::tuple
   getinitargs(const polygon_pattern_symbolizer& p)
    {
	std::string filename = path_processor_type::to_string(*p.get_filename());
	return boost::python::make_tuple(filename,guess_type(filename));
   }
};

void export_polygon_pattern_symbolizer()
{
    using namespace boost::python;
    
    class_<polygon_pattern_symbolizer>("PolygonPatternSymbolizer",
				       init<path_expression_ptr>("<path_expression_ptr>"))
        .def_pickle(polygon_pattern_symbolizer_pickle_suite())	;    
}
