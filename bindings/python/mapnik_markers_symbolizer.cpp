/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Artem Pavlenko
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

#include <mapnik/graphics.hpp>
#include <mapnik/value_error.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/markers_symbolizer.hpp>
#include <mapnik/parse_path.hpp>
#include "mapnik_svg.hpp"
#include "mapnik_enumeration.hpp"
#include <mapnik/marker_cache.hpp> // for known_svg_prefix_

using mapnik::markers_symbolizer;
using mapnik::symbolizer_with_image;
using mapnik::path_processor_type;
using mapnik::parse_path;

namespace  {
using namespace boost::python;

std::string get_filename(mapnik::markers_symbolizer const& symbolizer)
{
    return path_processor_type::to_string(*symbolizer.get_filename());
}

void set_filename(mapnik::markers_symbolizer & symbolizer, std::string const& file_expr)
{
    symbolizer.set_filename(parse_path(file_expr));
}

void set_marker_type(mapnik::markers_symbolizer & symbolizer, std::string const& marker_type)
{
    std::string filename;
    if (marker_type == "ellipse")
    {
        filename = mapnik::marker_cache::instance().known_svg_prefix_ + "ellipse";
    }
    else if (marker_type == "arrow")
    {
        filename = mapnik::marker_cache::instance().known_svg_prefix_ + "arrow";
    }
    else
    {
        throw mapnik::value_error("Unknown marker-type: '" + marker_type + "'");
    }
    symbolizer.set_filename(parse_path(filename));
}

}


// https://github.com/mapnik/mapnik/issues/1367
PyObject* get_fill_opacity_impl(markers_symbolizer & sym)
{
    boost::optional<float> fill_opacity = sym.get_fill_opacity();
    if (fill_opacity)
        return ::PyFloat_FromDouble(*fill_opacity);
    Py_RETURN_NONE;
}

void export_markers_symbolizer()
{
    using namespace boost::python;

    mapnik::enumeration_<mapnik::marker_placement_e>("marker_placement")
        .value("POINT_PLACEMENT",mapnik::MARKER_POINT_PLACEMENT)
        .value("INTERIOR_PLACEMENT",mapnik::MARKER_INTERIOR_PLACEMENT)
        .value("LINE_PLACEMENT",mapnik::MARKER_LINE_PLACEMENT)
        ;

    class_<markers_symbolizer>("MarkersSymbolizer",
                               init<>("Default Markers Symbolizer - circle"))
        .def (init<mapnik::path_expression_ptr>("<path expression ptr>"))
        .add_property("filename",
                      &get_filename,
                      &set_filename)
        .add_property("marker_type",
                      &get_filename,
                      &set_marker_type)
        .add_property("allow_overlap",
                      &markers_symbolizer::get_allow_overlap,
                      &markers_symbolizer::set_allow_overlap)
        .add_property("spacing",
                      &markers_symbolizer::get_spacing,
                      &markers_symbolizer::set_spacing)
        .add_property("max_error",
                      &markers_symbolizer::get_max_error,
                      &markers_symbolizer::set_max_error)
        .add_property("opacity",
                      &markers_symbolizer::get_opacity,
                      &markers_symbolizer::set_opacity,
                      "Set/get the overall opacity")
        .add_property("fill_opacity",
                      &get_fill_opacity_impl,
                      &markers_symbolizer::set_fill_opacity,
                      "Set/get the fill opacity")
        .add_property("ignore_placement",
                      &markers_symbolizer::get_ignore_placement,
                      &markers_symbolizer::set_ignore_placement)
        .add_property("transform",
                      &mapnik::get_svg_transform<markers_symbolizer>,
                      &mapnik::set_svg_transform<markers_symbolizer>)
        .add_property("width",
                      make_function(&markers_symbolizer::get_width,
                                    return_value_policy<copy_const_reference>()),
                      &markers_symbolizer::set_width,
                      "Set/get the marker width")
        .add_property("height",
                      make_function(&markers_symbolizer::get_height,
                                    return_value_policy<copy_const_reference>()),
                      &markers_symbolizer::set_height,
                      "Set/get the marker height")
        .add_property("fill",
                      &markers_symbolizer::get_fill,
                      &markers_symbolizer::set_fill,
                      "Set/get the marker fill color")
        .add_property("stroke",
                      &markers_symbolizer::get_stroke,
                      &markers_symbolizer::set_stroke,
                      "Set/get the marker stroke (outline)")
        .add_property("placement",
                      &markers_symbolizer::get_marker_placement,
                      &markers_symbolizer::set_marker_placement,
                      "Set/get the marker placement")
        .add_property("comp_op",
                      &markers_symbolizer::comp_op,
                      &markers_symbolizer::set_comp_op,
                      "Set/get the marker comp-op")
        .add_property("clip",
                      &markers_symbolizer::clip,
                      &markers_symbolizer::set_clip,
                      "Set/get the marker geometry's clipping status")
        .add_property("smooth",
                      &markers_symbolizer::smooth,
                      &markers_symbolizer::set_smooth,
                      "Set/get the marker geometry's smooth value")
        ;
}
