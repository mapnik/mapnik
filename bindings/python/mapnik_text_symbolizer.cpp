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

#include <mapnik/text_symbolizer.hpp>
#include "mapnik_enumeration.hpp"

void export_text_symbolizer()
{
   using namespace boost::python;
   using namespace mapnik;

   using mapnik::text_symbolizer;
   using mapnik::color;
 
   enumeration_<label_placement_e>("label_placement")
      .value("LINE_PLACEMENT",LINE_PLACEMENT)
      .value("POINT_PLACEMENT",POINT_PLACEMENT)
      ;
    
   class_<text_symbolizer>("TextSymbolizer",
                           init<std::string const&,std::string const&, unsigned,color const&>())
      .add_property("halo_fill",make_function(
                       &text_symbolizer::get_halo_fill,
                       return_value_policy<copy_const_reference>()),
                    &text_symbolizer::set_halo_fill)
      .add_property("halo_radius",
                    &text_symbolizer::get_halo_radius, 
                    &text_symbolizer::set_halo_radius)
      .add_property("wrap_width",
                    &text_symbolizer::get_wrap_width,
                    &text_symbolizer::set_wrap_width)
      .add_property("text_ratio",
                    &text_symbolizer::get_text_ratio,
                    &text_symbolizer::set_text_ratio)
      .add_property("label_spacing",
                    &text_symbolizer::get_label_spacing,
                    &text_symbolizer::set_label_spacing)
      .add_property("label_position_tolerance",
                    &text_symbolizer::get_label_position_tolerance,
                    &text_symbolizer::set_label_position_tolerance)
      .add_property("force_odd_labels",
                    &text_symbolizer::get_force_odd_labels,
                    &text_symbolizer::set_force_odd_labels)

      .add_property("fontset",
                    make_function(&text_symbolizer::get_fontset,return_value_policy<copy_const_reference>()),
                    &text_symbolizer::set_fontset)

      .add_property("fill",              
                    make_function(&text_symbolizer::get_fill,return_value_policy<copy_const_reference>()),
                    &text_symbolizer::set_fill)
      .add_property("name",
                    make_function(&text_symbolizer::get_name,return_value_policy<copy_const_reference>()),
                    &text_symbolizer::set_name)

      .add_property("text_size",
                    &text_symbolizer::get_text_size,
                    &text_symbolizer::set_text_size)

      .add_property("face_name",
                    make_function(&text_symbolizer::get_face_name,return_value_policy<copy_const_reference>()),
                    &text_symbolizer::set_face_name)
                    
      .add_property("max_char_angle_delta",
                    &text_symbolizer::get_max_char_angle_delta,
                    &text_symbolizer::set_max_char_angle_delta)
      .add_property("avoid_edges",
                    &text_symbolizer::get_avoid_edges,
                    &text_symbolizer::set_avoid_edges)
      .add_property("minimum_distance",
                    &text_symbolizer::get_minimum_distance,
                    &text_symbolizer::set_minimum_distance)
      .def("displacement",&text_symbolizer::set_displacement)
      .def("anchor",&text_symbolizer::set_anchor)
      .add_property("label_placement",
                    &text_symbolizer::get_label_placement,
                    &text_symbolizer::set_label_placement,
                    "Set/get the placement of the label")
      .add_property("allow_overlap",
                    &text_symbolizer::get_allow_overlap,
                    &text_symbolizer::set_allow_overlap,
                    "Set/get the allow_overlap property of the label")
      ;
}
