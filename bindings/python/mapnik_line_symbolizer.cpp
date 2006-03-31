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
#include <mapnik.hpp>

using mapnik::line_symbolizer;
using mapnik::stroke;
using mapnik::Color;

void export_line_symbolizer()
{
    using namespace boost::python;
    
    class_<line_symbolizer>("LineSymbolizer",init<stroke const&>("TODO"))
	.def(init<Color const& ,float>())
	.add_property("stroke",make_function
		      (&line_symbolizer::get_stroke,return_value_policy<reference_existing_object>()),
		      &line_symbolizer::set_stroke)
	;    
}
