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
    s.set_shield_displacement(extract<double>(arg[0]),extract<double>(arg[1]));
}

tuple get_text_displacement(const shield_symbolizer& t)
{
    position const& pos = t.get_displacement();
    return boost::python::make_tuple(pos.first, pos.second);
}

void set_text_displacement(shield_symbolizer & t, boost::python::tuple arg)
{
    t.set_displacement(extract<double>(arg[0]),extract<double>(arg[1]));
}

const std::string get_filename(shield_symbolizer const& t)
{
    return path_processor_type::to_string(*t.get_filename());
}

void set_filename(shield_symbolizer & t, std::string const& file_expr)
{
    t.set_filename(parse_path(file_expr));
}

}

struct shield_symbolizer_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const shield_symbolizer& s)
    {
        std::string filename = path_processor_type::to_string(*s.get_filename());
        //(name, font name, font size, font color, image file, image type, width, height)
        return boost::python::make_tuple( "TODO",//s.get_name(),
                                          s.get_face_name(),s.get_text_size(),s.get_fill(),filename,guess_type(filename));

    }

    static  boost::python::tuple
    getstate(const shield_symbolizer& s)
    {
        return boost::python::make_tuple(s.get_halo_fill(),s.get_halo_radius());
    }

    // TODO add lots more...
    static void
    setstate (shield_symbolizer& s, boost::python::tuple state)
    {
        using namespace boost::python;
        /*if (len(state) != 1)
          {
          PyErr_SetObject(PyExc_ValueError,
          ("expected 1-item tuple in call to __setstate__; got %s"
          % state).ptr()
          );
          throw_error_already_set();
          }*/

        s.set_halo_fill(extract<color>(state[0]));
        s.set_halo_radius(extract<float>(state[1]));

    }

};


void export_shield_symbolizer()
{
    using namespace boost::python;
    class_< shield_symbolizer, bases<text_symbolizer> >("ShieldSymbolizer",
                                                        init<expression_ptr,
                                                        std::string const&,
                                                        unsigned, mapnik::color const&,
                                                        path_expression_ptr>("TODO")
        )
        //.def_pickle(shield_symbolizer_pickle_suite())
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
        ;
}
