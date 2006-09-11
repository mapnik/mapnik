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
#include <text_symbolizer.hpp>

void export_text_symbolizer()
{
    using namespace boost::python;

    using mapnik::text_symbolizer;
    using mapnik::Color;
 
    class_<text_symbolizer>("TextSymbolizer",
			    init<std::string const&,unsigned,Color const&>())
        .add_property("halo_fill",make_function(
                      &text_symbolizer::get_halo_fill,
                      return_value_policy<copy_const_reference>()),
                      &text_symbolizer::set_halo_fill)
        .add_property("halo_radius",
                      &text_symbolizer::get_halo_radius, 
                      &text_symbolizer::set_halo_radius)
	;
}
