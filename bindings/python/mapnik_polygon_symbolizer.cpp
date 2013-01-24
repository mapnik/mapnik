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
#include <mapnik/polygon_symbolizer.hpp>

using namespace mapnik;
using mapnik::polygon_symbolizer;
using mapnik::color;

void export_polygon_symbolizer()
{
    using namespace boost::python;

    class_<polygon_symbolizer>("PolygonSymbolizer",
                               init<>("Default PolygonSymbolizer - solid fill grey"))
        .def(init<color const&>("TODO"))
        .add_property("fill",make_function
                      (&polygon_symbolizer::get_fill,
                       return_value_policy<copy_const_reference>()),
                      &polygon_symbolizer::set_fill)
        .add_property("fill_opacity",
                      &polygon_symbolizer::get_opacity,
                      &polygon_symbolizer::set_opacity)
        .add_property("gamma",
                      &polygon_symbolizer::get_gamma,
                      &polygon_symbolizer::set_gamma)
        .add_property("gamma_method",
                      &polygon_symbolizer::get_gamma_method,
                      &polygon_symbolizer::set_gamma_method,
                      "gamma correction method")
        .add_property("comp_op",
                      &polygon_symbolizer::comp_op,
                      &polygon_symbolizer::set_comp_op,
                      "Set/get the polygon comp-op")
        .add_property("clip",
                      &polygon_symbolizer::clip,
                      &polygon_symbolizer::set_clip,
                      "Set/get the polygon geometry's clipping status")
        .add_property("smooth",
                      &polygon_symbolizer::smooth,
                      &polygon_symbolizer::set_smooth,
                      "Set/get the polygon geometry's smooth value")
        .add_property("simplify_tolerance",
                      &polygon_symbolizer::simplify_tolerance,
                      &polygon_symbolizer::set_simplify_tolerance,
                      "simplfication tolerance measure")
        ;

}
