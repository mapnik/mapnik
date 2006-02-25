/* This file is part of python_mapnik (c++/python mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko, Jean-Francois Doyon
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

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
