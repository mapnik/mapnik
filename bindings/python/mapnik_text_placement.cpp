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

#include <mapnik/text_placements.hpp>
#include "mapnik_enumeration.hpp"
#include <mapnik/expression_string.hpp>

using namespace mapnik;
//using mapnik::color;
//using mapnik::text_symbolizer;
//using mapnik::expr_node;
//using mapnik::expression_ptr;
//using mapnik::to_expression_string;

namespace {
using namespace boost::python;

tuple get_displacement(text_symbolizer_properties const& t)
{
    return boost::python::make_tuple(t.displacement.first, t.displacement.second);
}

void set_displacement(text_symbolizer_properties &t, boost::python::tuple arg)
{
    double x = extract<double>(arg[0]);
    double y = extract<double>(arg[0]);
    t.displacement = std::make_pair(x, y);
}

}

void export_text_placement()
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
        .value("AUTO",H_AUTO)
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


    class_<text_symbolizer_properties>("TextSymbolizerProperties")
        .def_readwrite("orientation", &text_symbolizer_properties::orientation)
        .add_property("displacement",
                       &get_displacement,
                       &set_displacement)
        .def_readwrite("label_placement", &text_symbolizer_properties::label_placement)
        .def_readwrite("horizontal_alignment", &text_symbolizer_properties::halign)
        .def_readwrite("justify_alignment", &text_symbolizer_properties::jalign)
        .def_readwrite("vertical_alignment", &text_symbolizer_properties::valign)
        .def_readwrite("label_spacing", &text_symbolizer_properties::label_spacing)
        .def_readwrite("label_position_tolerance", &text_symbolizer_properties::label_position_tolerance)
        .def_readwrite("avoid_edges", &text_symbolizer_properties::avoid_edges)
        .def_readwrite("minimum_distance", &text_symbolizer_properties::minimum_distance)
        .def_readwrite("minimum_padding", &text_symbolizer_properties::minimum_padding)
        .def_readwrite("minimum_path_length", &text_symbolizer_properties::minimum_path_length)
        .def_readwrite("maximum_angle_char_delta", &text_symbolizer_properties::max_char_angle_delta)
        .def_readwrite("force_odd_labels", &text_symbolizer_properties::force_odd_labels)
        .def_readwrite("allow_overlap", &text_symbolizer_properties::allow_overlap)
        .def_readwrite("text_ratio", &text_symbolizer_properties::text_ratio)
        .def_readwrite("wrap_width", &text_symbolizer_properties::wrap_width)
        /* TODO: text_processor */
        /* from_xml, to_xml operate on mapnik's internal XML tree and don't make sense in python.*/
        ;
}
