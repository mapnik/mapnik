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
#include <mapnik/expression_string.hpp>

using namespace mapnik;
using mapnik::color;
using mapnik::text_symbolizer;
using mapnik::expr_node;
using mapnik::expression_ptr;
using mapnik::to_expression_string;

namespace {
using namespace boost::python;

tuple get_text_displacement(const text_symbolizer& t)
{
    position pos = t.get_displacement();
    return boost::python::make_tuple(boost::get<0>(pos),boost::get<1>(pos));
}

void set_text_displacement(text_symbolizer & t, boost::python::tuple arg)
{
    t.set_displacement(extract<double>(arg[0]),extract<double>(arg[1]));
}

tuple get_anchor(const text_symbolizer& t)
{
    position pos = t.get_anchor();
    return boost::python::make_tuple(boost::get<0>(pos),boost::get<1>(pos));
}

void set_anchor(text_symbolizer & t, boost::python::tuple arg)
{
    t.set_anchor(extract<double>(arg[0]),extract<double>(arg[1]));
}

void set_placement_options(text_symbolizer & t, placement_type_e arg, std::string const& placements)
{
    t.set_placement_options(arg, placements);
}

void set_placement_options_2(text_symbolizer & t, placement_type_e arg)
{
    t.set_placement_options(arg, "");
}

}

struct text_symbolizer_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(const text_symbolizer& t)
    {

        return boost::python::make_tuple("TODO",//t.get_name(),
                                         t.get_face_name(),t.get_text_size(),t.get_fill());

    }

    static  boost::python::tuple
    getstate(const text_symbolizer& t)
    {
        boost::python::tuple disp = get_text_displacement(t);
        boost::python::tuple anchor = get_anchor(t);

        // so we do not exceed max args accepted by make_tuple,
        // lets put the increasing list of parameters in a list
        boost::python::list extras;
        extras.append(t.get_wrap_char_string());
        extras.append(t.get_line_spacing());
        extras.append(t.get_character_spacing());
        extras.append(t.get_text_transform());
        extras.append(t.get_wrap_before());
        extras.append(t.get_horizontal_alignment());
        extras.append(t.get_justify_alignment());
        extras.append(t.get_text_opacity());
        extras.append(t.get_minimum_padding());
        extras.append(t.get_minimum_path_length());

        return boost::python::make_tuple(disp,t.get_label_placement(),
                                         t.get_vertical_alignment(),t.get_halo_radius(),t.get_halo_fill(),t.get_text_ratio(),
                                         t.get_wrap_width(),t.get_label_spacing(),t.get_minimum_distance(),t.get_allow_overlap(),
                                         anchor,t.get_force_odd_labels(),t.get_max_char_angle_delta(),extras
            );
    }

    static void
    setstate (text_symbolizer& t, boost::python::tuple state)
    {
        using namespace boost::python;

        if (len(state) != 14)
        {
            PyErr_SetObject(PyExc_ValueError,
                            ("expected 15-item tuple in call to __setstate__; got %s"
                             % state).ptr()
                );
            throw_error_already_set();
        }

        tuple disp = extract<tuple>(state[0]);
        double dx = extract<double>(disp[0]);
        double dy = extract<double>(disp[1]);
        t.set_displacement(dx,dy);

        t.set_label_placement(extract<label_placement_e>(state[1]));

        t.set_vertical_alignment(extract<vertical_alignment_e>(state[2]));

        t.set_halo_radius(extract<unsigned>(state[3]));

        t.set_halo_fill(extract<color>(state[4]));

        t.set_text_ratio(extract<unsigned>(state[5]));

        t.set_wrap_width(extract<unsigned>(state[6]));

        t.set_label_spacing(extract<unsigned>(state[7]));

        t.set_minimum_distance(extract<double>(state[8]));

        t.set_allow_overlap(extract<bool>(state[9]));

        tuple anch = extract<tuple>(state[10]);
        double x = extract<double>(anch[0]);
        double y = extract<double>(anch[1]);
        t.set_anchor(x,y);

        t.set_force_odd_labels(extract<bool>(state[11]));

        t.set_max_char_angle_delta(extract<double>(state[12]));
        list extras = extract<list>(state[13]);
        t.set_wrap_char_from_string(extract<std::string>(extras[0]));
        t.set_line_spacing(extract<unsigned>(extras[1]));
        t.set_character_spacing(extract<unsigned>(extras[2]));
        t.set_text_transform(extract<text_transform_e>(extras[3]));
        t.set_wrap_before(extract<bool>(extras[4]));
        t.set_horizontal_alignment(extract<horizontal_alignment_e>(extras[5]));
        t.set_justify_alignment(extract<justify_alignment_e>(extras[6]));
        t.set_text_opacity(extract<double>(extras[7]));
        t.set_minimum_padding(extract<double>(extras[8]));
        t.set_minimum_path_length(extract<double>(extras[9]));
    }
};


void export_text_symbolizer()
{
    using namespace boost::python;

    enumeration_<label_placement_e>("label_placement")
        .value("LINE_PLACEMENT",LINE_PLACEMENT)
        .value("POINT_PLACEMENT",POINT_PLACEMENT)
        .value("VERTEX_PLACEMENT",VERTEX_PLACEMENT)
        .value("INTERIOR_PLACEMENT",INTERIOR_PLACEMENT)
        ;
    enumeration_<vertical_alignment_e>("vertical_alignment")
        .value("TOP",V_TOP)
        .value("MIDDLE",V_MIDDLE)
        .value("BOTTOM",V_BOTTOM)
        .value("AUTO",V_AUTO)
        ;

    enumeration_<horizontal_alignment_e>("horizontal_alignment")
        .value("LEFT",H_LEFT)
        .value("MIDDLE",H_MIDDLE)
        .value("RIGHT",H_RIGHT)
        ;

    enumeration_<justify_alignment_e>("justify_alignment")
        .value("LEFT",J_LEFT)
        .value("MIDDLE",J_MIDDLE)
        .value("RIGHT",J_RIGHT)
        ;

    enumeration_<text_transform_e>("text_transform")
        .value("NONE",NONE)
        .value("UPPERCASE",UPPERCASE)
        .value("LOWERCASE",LOWERCASE)
        .value("CAPITALIZE",CAPITALIZE)
        ;

    enumeration_<placement_type_e>("placement_type")
        .value("SIMPLE",T_SIMPLE)
        .value("DUMMY",T_DUMMY)
        ;

    class_<text_symbolizer>("TextSymbolizer",init<expression_ptr,std::string const&, unsigned,color const&>())
        /*
        // todo - all python classes can have kwargs and default constructors
        class_<text_symbolizer>("TextSymbolizer",
        init<expression_ptr,std::string const&, unsigned,color const&>(
        (
        arg("name"),
        arg("font_face")="DejaVu Sans Book",
        arg("size")=10,
        arg("color")=color("black")
        ),
        "Create a TextSymbolizer\n"
        ))
        */

        //.def_pickle(text_symbolizer_pickle_suite())
        .add_property("anchor",
                      &get_anchor,
                      &set_anchor)
        .add_property("allow_overlap",
                      &text_symbolizer::get_allow_overlap,
                      &text_symbolizer::set_allow_overlap,
                      "Set/get the allow_overlap property of the label")
        .add_property("displacement",
                      &get_text_displacement,
                      &set_text_displacement)
        .add_property("avoid_edges",
                      &text_symbolizer::get_avoid_edges,
                      &text_symbolizer::set_avoid_edges,
                      "Set/get the avoid_edge property of the label")
        .add_property("character_spacing",
                      &text_symbolizer::get_character_spacing,
                      &text_symbolizer::set_character_spacing,
                      "Set/get the character_spacing property of the label")
        .add_property("face_name",
                      make_function(&text_symbolizer::get_face_name,return_value_policy<copy_const_reference>()),
                      &text_symbolizer::set_face_name,
                      "Set/get the face_name property of the label")
        .add_property("fill",
                      make_function(&text_symbolizer::get_fill,return_value_policy<copy_const_reference>()),
                      &text_symbolizer::set_fill)
        .add_property("fontset",
                      make_function(&text_symbolizer::get_fontset,return_value_policy<copy_const_reference>()),
                      &text_symbolizer::set_fontset)
        .add_property("force_odd_labels",
                      &text_symbolizer::get_force_odd_labels,
                      &text_symbolizer::set_force_odd_labels)
        .add_property("halo_fill",
                      make_function(&text_symbolizer::get_halo_fill,return_value_policy<copy_const_reference>()),
                      &text_symbolizer::set_halo_fill)
        .add_property("halo_radius",
                      &text_symbolizer::get_halo_radius,
                      &text_symbolizer::set_halo_radius)
        .add_property("horizontal_alignment",
                      &text_symbolizer::get_horizontal_alignment,
                      &text_symbolizer::set_horizontal_alignment,
                      "Set/get the horizontal alignment of the label")
        .add_property("justify_alignment",
                      &text_symbolizer::get_justify_alignment,
                      &text_symbolizer::set_justify_alignment,
                      "Set/get the text justification")
        .add_property("label_placement",
                      &text_symbolizer::get_label_placement,
                      &text_symbolizer::set_label_placement,
                      "Set/get the placement of the label")
        .add_property("label_position_tolerance",
                      &text_symbolizer::get_label_position_tolerance,
                      &text_symbolizer::set_label_position_tolerance)
        .add_property("label_spacing",
                      &text_symbolizer::get_label_spacing,
                      &text_symbolizer::set_label_spacing)
        .add_property("line_spacing",
                      &text_symbolizer::get_line_spacing,
                      &text_symbolizer::set_line_spacing)
        .add_property("max_char_angle_delta",
                      &text_symbolizer::get_max_char_angle_delta,
                      &text_symbolizer::set_max_char_angle_delta)
        .add_property("minimum_distance",
                      &text_symbolizer::get_minimum_distance,
                      &text_symbolizer::set_minimum_distance)
        .add_property("minimum_padding",
                      &text_symbolizer::get_minimum_padding,
                      &text_symbolizer::set_minimum_padding)
        .add_property("minimum_path_length",
                      &text_symbolizer::get_minimum_path_length,
                      &text_symbolizer::set_minimum_path_length)
        .add_property("name",&text_symbolizer::get_name,
                      &text_symbolizer::set_name)
        .add_property("opacity",
                      &text_symbolizer::get_text_opacity,
                      &text_symbolizer::set_text_opacity,
                      "Set/get the text opacity")
        .add_property("placement",
                      &text_symbolizer::get_label_placement,
                      &text_symbolizer::set_label_placement,
                      "Set/get the placement of the label")
        .add_property("text_transform",
                      &text_symbolizer::get_text_transform,
                      &text_symbolizer::set_text_transform,
                      "Set/get the text conversion method")
        .add_property("text_ratio",
                      &text_symbolizer::get_text_ratio,
                      &text_symbolizer::set_text_ratio)
        .add_property("text_size",
                      &text_symbolizer::get_text_size,
                      &text_symbolizer::set_text_size)
        .add_property("vertical_alignment",
                      &text_symbolizer::get_vertical_alignment,
                      &text_symbolizer::set_vertical_alignment,
                      "Set/get the vertical alignment of the label")
        .add_property("wrap_width",
                      &text_symbolizer::get_wrap_width,
                      &text_symbolizer::set_wrap_width)
        .add_property("wrap_character",
                      &text_symbolizer::get_wrap_char_string,
                      &text_symbolizer::set_wrap_char_from_string)
        .add_property("wrap_before",
                      &text_symbolizer::get_wrap_before,
                      &text_symbolizer::set_wrap_before)
        .add_property("placement_type", &text_symbolizer::get_placement_type)
        .add_property("placements", &text_symbolizer::get_placements)
        .def("set_placement_options", set_placement_options)
        .def("set_placement_options", set_placement_options_2)
        ;
}
