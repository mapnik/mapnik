/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko, Jean-Francois Doyon
 * Copyright (C) 2006 10East Corp.
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

/* The functions in this file produce deprecation warnings.
 * But as shield symbolizer doesn't fully support more than one
 * placement from python yet these functions are actually the
 * correct ones.
 */
#define NO_DEPRECATION_WARNINGS

// boost
#include <boost/python.hpp>

// mapnik
#include <mapnik/shield_symbolizer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/path_expression_grammar.hpp>
#include "mapnik_svg.hpp"

using mapnik::color;
using mapnik::shield_symbolizer;
using mapnik::text_symbolizer;
using mapnik::symbolizer_with_image;
using mapnik::path_processor_type;
using mapnik::path_expression_ptr;
using mapnik::guess_type;
using mapnik::expression_ptr;
using mapnik::parse_path;
using mapnik::position;


namespace {
using namespace boost::python;

tuple get_shield_displacement(const shield_symbolizer& s)
{
    position const& pos = s.get_shield_displacement();
    return boost::python::make_tuple(pos.first, pos.second);
}

void set_shield_displacement(shield_symbolizer & s, boost::python::tuple arg)
{
    s.get_placement_options()->defaults.displacement.first = extract<double>(arg[0]);
    s.get_placement_options()->defaults.displacement.second = extract<double>(arg[1]);
}

tuple get_text_displacement(const shield_symbolizer& t)
{
    position const& pos = t.get_placement_options()->defaults.displacement;
    return boost::python::make_tuple(pos.first, pos.second);
}

void set_text_displacement(shield_symbolizer & t, boost::python::tuple arg)
{
    t.set_displacement(extract<double>(arg[0]),extract<double>(arg[1]));
}

std::string get_filename(shield_symbolizer const& t)
{
    return path_processor_type::to_string(*t.get_filename());
}

void set_filename(shield_symbolizer & t, std::string const& file_expr)
{
    t.set_filename(parse_path(file_expr));
}

}

void export_shield_symbolizer()
{
    using namespace boost::python;
    class_< shield_symbolizer, bases<text_symbolizer> >("ShieldSymbolizer",
                                                        init<expression_ptr,
                                                        std::string const&,
                                                        unsigned, mapnik::color const&,
                                                        path_expression_ptr>()
        )
        .add_property("allow_overlap",
                      &shield_symbolizer::get_allow_overlap,
                      &shield_symbolizer::set_allow_overlap,
                      "Set/get the allow_overlap property of the label")
        .add_property("avoid_edges",
                      &shield_symbolizer::get_avoid_edges,
                      &shield_symbolizer::set_avoid_edges,
                      "Set/get the avoid_edge property of the label")
        .add_property("character_spacing",
                      &shield_symbolizer::get_character_spacing,
                      &shield_symbolizer::set_character_spacing,
                      "Set/get the character_spacing property of the label")
        .add_property("displacement",
                      &get_text_displacement,
                      &set_text_displacement)
        .add_property("face_name",
                      make_function(&shield_symbolizer::get_face_name,return_value_policy<copy_const_reference>()),
                      &shield_symbolizer::set_face_name,
                      "Set/get the face_name property of the label")
        .add_property("fill",
                      make_function(&shield_symbolizer::get_fill,return_value_policy<copy_const_reference>()),
                      &shield_symbolizer::set_fill)
        .add_property("fontset",
                      make_function(&shield_symbolizer::get_fontset,return_value_policy<copy_const_reference>()),
                      &shield_symbolizer::set_fontset)
        .add_property("force_odd_labels",
                      &shield_symbolizer::get_force_odd_labels,
                      &shield_symbolizer::set_force_odd_labels)
        .add_property("halo_fill",
                      make_function(&shield_symbolizer::get_halo_fill,return_value_policy<copy_const_reference>()),
                      &shield_symbolizer::set_halo_fill)
        .add_property("halo_radius",
                      &shield_symbolizer::get_halo_radius,
                      &shield_symbolizer::set_halo_radius)
        .add_property("horizontal_alignment",
                      &shield_symbolizer::get_horizontal_alignment,
                      &shield_symbolizer::set_horizontal_alignment,
                      "Set/get the horizontal alignment of the label")
        .add_property("justify_alignment",
                      &shield_symbolizer::get_justify_alignment,
                      &shield_symbolizer::set_justify_alignment,
                      "Set/get the text justification")
        .add_property("label_placement",
                      &shield_symbolizer::get_label_placement,
                      &shield_symbolizer::set_label_placement,
                      "Set/get the placement of the label")
        .add_property("label_position_tolerance",
                      &shield_symbolizer::get_label_position_tolerance,
                      &shield_symbolizer::set_label_position_tolerance)
        .add_property("label_spacing",
                      &shield_symbolizer::get_label_spacing,
                      &shield_symbolizer::set_label_spacing)
        .add_property("line_spacing",
                      &shield_symbolizer::get_line_spacing,
                      &shield_symbolizer::set_line_spacing)
        .add_property("max_char_angle_delta",
                      &shield_symbolizer::get_max_char_angle_delta,
                      &shield_symbolizer::set_max_char_angle_delta)
        .add_property("minimum_distance",
                      &shield_symbolizer::get_minimum_distance,
                      &shield_symbolizer::set_minimum_distance)
        .add_property("minimum_padding",
                      &shield_symbolizer::get_minimum_padding,
                      &shield_symbolizer::set_minimum_padding)
        .add_property("name",&shield_symbolizer::get_name,
                      &shield_symbolizer::set_name)
        .add_property("opacity",
                      &shield_symbolizer::get_opacity,
                      &shield_symbolizer::set_opacity,
                      "Set/get the shield opacity")
        .add_property("shield_displacement",
                      get_shield_displacement,
                      set_shield_displacement)
        .add_property("text_opacity",
                      &shield_symbolizer::get_text_opacity,
                      &shield_symbolizer::set_text_opacity,
                      "Set/get the text opacity")
        .add_property("text_transform",
                      &shield_symbolizer::get_text_transform,
                      &shield_symbolizer::set_text_transform,
                      "Set/get the text conversion method")
        .add_property("text_ratio",
                      &shield_symbolizer::get_text_ratio,
                      &shield_symbolizer::set_text_ratio)
        .add_property("text_size",
                      &shield_symbolizer::get_text_size,
                      &shield_symbolizer::set_text_size)
        .add_property("vertical_alignment",
                      &shield_symbolizer::get_vertical_alignment,
                      &shield_symbolizer::set_vertical_alignment,
                      "Set/get the vertical alignment of the label")
        .add_property("wrap_width",
                      &shield_symbolizer::get_wrap_width,
                      &shield_symbolizer::set_wrap_width)
        .add_property("wrap_character",
                      &shield_symbolizer::get_wrap_char_string,
                      &shield_symbolizer::set_wrap_char_from_string)
        .add_property("wrap_before",
                      &shield_symbolizer::get_wrap_before,
                      &shield_symbolizer::set_wrap_before)
        .add_property("unlock_image",
                      &shield_symbolizer::get_unlock_image,
                      &shield_symbolizer::set_unlock_image)
        .add_property("filename",
                      &get_filename,
                      &set_filename)
        .add_property("transform",
                      mapnik::get_svg_transform<shield_symbolizer>,
                      mapnik::set_svg_transform<shield_symbolizer>)
        .add_property("comp_op",
                      &shield_symbolizer::comp_op,
                      &shield_symbolizer::set_comp_op,
                      "Set/get the comp-op")
        .add_property("clip",
                      &shield_symbolizer::clip,
                      &shield_symbolizer::set_clip,
                      "Set/get the shield geometry's clipping status")
        ;
}
