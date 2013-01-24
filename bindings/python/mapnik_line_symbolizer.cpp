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

#include <boost/python.hpp>
#include "mapnik_enumeration.hpp"
#include <mapnik/line_symbolizer.hpp>

using namespace mapnik;
using mapnik::line_symbolizer;
using mapnik::stroke;
using mapnik::color;

void export_line_symbolizer()
{
    using namespace boost::python;
    enumeration_<line_rasterizer_e>("line_rasterizer")
        .value("FULL",RASTERIZER_FULL)
        .value("FAST",RASTERIZER_FAST)
        ;
    class_<line_symbolizer>("LineSymbolizer",
                            init<>("Default LineSymbolizer - 1px solid black"))
        .def(init<stroke const&>("TODO"))
        .def(init<color const& ,float>())
        .add_property("rasterizer",
                      &line_symbolizer::get_rasterizer,
                      &line_symbolizer::set_rasterizer,
                      "Set/get the rasterization method of the line of the point")
        .add_property("stroke",make_function
                      (&line_symbolizer::get_stroke,
                       return_value_policy<reference_existing_object>()),
                      &line_symbolizer::set_stroke)
        .add_property("simplify_tolerance",
                      &line_symbolizer::simplify_tolerance,
                      &line_symbolizer::set_simplify_tolerance,
                      "simplification tolerance measure")
        .add_property("offset",
                      &line_symbolizer::offset,
                      &line_symbolizer::set_offset,
                      "offset value")
        .add_property("comp_op",
                      &line_symbolizer::comp_op,
                      &line_symbolizer::set_comp_op,
                      "Set/get the comp-op")
        .add_property("clip",
                      &line_symbolizer::clip,
                      &line_symbolizer::set_clip,
                      "Set/get the line geometry's clipping status")
        .add_property("smooth",
                      &line_symbolizer::smooth,
                      &line_symbolizer::set_smooth,
                      "smooth value (0..1.0)")
        ;
}
