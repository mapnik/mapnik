/*****************************************************************************
 *
 * This file is part of Mapnik (c+mapping toolkit)
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
#include <mapnik/stroke.hpp>
#include "mapnik_enumeration.hpp"

using namespace mapnik;

namespace {
using namespace boost::python;

list get_dashes_list(stroke const& stroke)
{
    list l;
    if (stroke.has_dash()) {
        mapnik::dash_array const& dash = stroke.get_dash_array();
        mapnik::dash_array::const_iterator iter = dash.begin();
        mapnik::dash_array::const_iterator end = dash.end();
        for (; iter != end; ++iter) {
            l.append(make_tuple(iter->first, iter->second));
        }
    }
    return l;
}

void set_dasharray(stroke & stroke, list const& l)
{
    for (int i=0; i<len(l); ++i)
    {
        boost::python::tuple dash = extract<boost::python::tuple>(l[i]);
        if (len(dash) == 2)
        {
            double d1 = extract<double>(dash[0]);
            double d2 = extract<double>(dash[1]);
            stroke.add_dash(d1,d2);
        }
    }
}

}

void export_stroke ()
{
    using namespace boost::python;

    enumeration_<line_cap_e>("line_cap",
                             "The possible values for a line cap used when drawing\n"
                             "with a stroke.\n")
        .value("BUTT_CAP",BUTT_CAP)
        .value("SQUARE_CAP",SQUARE_CAP)
        .value("ROUND_CAP",ROUND_CAP)
        ;
    enumeration_<line_join_e>("line_join",
                              "The possible values for the line joining mode\n"
                              "when drawing with a stroke.\n")
        .value("MITER_JOIN",MITER_JOIN)
        .value("MITER_REVERT_JOIN",MITER_REVERT_JOIN)
        .value("ROUND_JOIN",ROUND_JOIN)
        .value("BEVEL_JOIN",BEVEL_JOIN)
        ;

    class_<stroke>("Stroke",init<>(
                       "Creates a new default black stroke with the width of 1.\n"))
        .def(init<color,float>(
                 (arg("color"),arg("width")),
                 "Creates a new stroke object with a specified color and width.\n")
            )
        .add_property("color",make_function
                      (&stroke::get_color,return_value_policy<copy_const_reference>()),
                      &stroke::set_color,
                      "Gets or sets the stroke color.\n"
                      "Returns a new Color object on retrieval.\n")
        .add_property("width",
                      &stroke::get_width,
                      &stroke::set_width,
                      "Gets or sets the stroke width in pixels.\n")
        .add_property("opacity",
                      &stroke::get_opacity,
                      &stroke::set_opacity,
                      "Gets or sets the opacity of this stroke.\n"
                      "The value is a float between 0 and 1.\n")
        .add_property("gamma",
                      &stroke::get_gamma,
                      &stroke::set_gamma,
                      "Gets or sets the gamma of this stroke.\n"
                      "The value is a float between 0 and 1.\n")
        .add_property("gamma_method",
                      &stroke::get_gamma_method,
                      &stroke::set_gamma_method,
                      "Set/get the gamma correction method of this stroke")
        .add_property("line_cap",
                      &stroke::get_line_cap,
                      &stroke::set_line_cap,
                      "Gets or sets the line cap of this stroke. (alias of linecap)\n")
        .add_property("linecap",
                      &stroke::get_line_cap,
                      &stroke::set_line_cap,
                      "Gets or sets the linecap of this stroke.\n")
        .add_property("line_join",
                      &stroke::get_line_join,
                      &stroke::set_line_join,
                      "Returns the line join mode of this stroke. (alias of linejoin)\n")
        .add_property("linejoin",
                      &stroke::get_line_join,
                      &stroke::set_line_join,
                      "Returns the linejoin mode of this stroke.\n")
        .add_property("miterlimit",
                      &stroke::get_miterlimit,
                      &stroke::set_miterlimit,
                      "Returns the miterlimit mode of this stroke.\n")
        .def("add_dash",&stroke::add_dash,
             (arg("length"),arg("gap")),
             "Adds a dash segment to the dash patterns of this stroke.\n")
        .def("get_dashes", get_dashes_list,
             "Returns the list of dash segments for this stroke.\n")
        .add_property("dasharray",
                      get_dashes_list,
                      set_dasharray,
                      "Gets or sets dasharray string of this stroke. (alternate property to add_dash/get_dashes)\n")
        .add_property("dash_offset",
                      &stroke::dash_offset,
                      &stroke::set_dash_offset,
                      "Gets or sets dash offset of this stroke. (alias of dashoffet)\n")
        .add_property("dashoffset",
                      &stroke::dash_offset,
                      &stroke::set_dash_offset,
                      "Gets or sets dash offset of this stroke.\n")
        ;
}
