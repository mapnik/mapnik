/* This file is part of python_mapnik (c++/python mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
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

#include <mapnik.hpp>
#include <boost/python.hpp>


void export_stroke ()
{
    using namespace mapnik;
    using namespace boost::python;

    enum_<line_cap_e>("line_cap")
	.value("BUTT_CAP",BUTT_CAP)
	.value("SQUARE_CAP",SQUARE_CAP)
	.value("ROUND_CAP",ROUND_CAP)
	;
    enum_<line_join_e>("line_join")
	.value("MITER_JOIN",MITER_JOIN)
	.value("MITER_REVERT_JOIN",MITER_REVERT_JOIN)
	.value("ROUND_JOIN",ROUND_JOIN)
	.value("BEVEL_JOIN",BEVEL_JOIN)
	;

    class_<stroke>("stroke",init<>())
	.def(init<Color,float>())
	.add_property("color",make_function
		      (&stroke::get_color,return_value_policy<reference_existing_object>()),
		      &stroke::set_color)
	.add_property("width",&stroke::get_width,&stroke::set_width) 
	.add_property("opacity",&stroke::get_opacity,&stroke::set_opacity)
	.add_property("line_cap",&stroke::get_line_cap,&stroke::set_line_cap)
	.add_property("line_join",&stroke::get_line_join,&stroke::set_line_join)
	.def("add_dash",&stroke::add_dash)
	;
}
