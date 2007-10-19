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

#include <mapnik/stroke.hpp>
#include "mapnik_enumeration.hpp"

void export_stroke ()
{
    using namespace mapnik;
    using namespace boost::python;

    enumeration_<line_cap_e>("line_cap")
        .value("BUTT_CAP",BUTT_CAP)
        .value("SQUARE_CAP",SQUARE_CAP)
        .value("ROUND_CAP",ROUND_CAP)
        ;
    enumeration_<line_join_e>("line_join")
        .value("MITER_JOIN",MITER_JOIN)
        .value("MITER_REVERT_JOIN",MITER_REVERT_JOIN)
        .value("ROUND_JOIN",ROUND_JOIN)
        .value("BEVEL_JOIN",BEVEL_JOIN)
        ;

    class_<stroke>("Stroke",init<>())
        .def(init<Color,float>())
        .add_property("color",make_function
                      (&stroke::get_color,return_value_policy<copy_const_reference>()),
                      &stroke::set_color)
        .add_property("width",&stroke::get_width,&stroke::set_width) 
        .add_property("opacity",&stroke::get_opacity,&stroke::set_opacity)
        .add_property("line_cap",&stroke::get_line_cap,&stroke::set_line_cap)
        .add_property("line_join",&stroke::get_line_join,&stroke::set_line_join)
        .def("add_dash",&stroke::add_dash)
        ;
}
