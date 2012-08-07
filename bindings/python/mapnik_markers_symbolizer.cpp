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
#include <mapnik/image_util.hpp>
#include <mapnik/markers_symbolizer.hpp>
#include <mapnik/parse_path.hpp>
#include "mapnik_svg.hpp"
#include "mapnik_enumeration.hpp"

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

}

struct markers_symbolizer_pickle_suite : boost::python::pickle_suite
{
    static boost::python::tuple
    getinitargs(markers_symbolizer const& p)
    {
        std::string filename = path_processor_type::to_string(*p.get_filename());
        return boost::python::make_tuple(filename,mapnik::guess_type(filename));
    }

    static  boost::python::tuple
    getstate(markers_symbolizer const& p)
    {
        return boost::python::make_tuple(p.get_allow_overlap(),
                                         p.get_ignore_placement());//,p.get_opacity());
    }

    static void
    setstate (markers_symbolizer& p, boost::python::tuple state)
    {
        using namespace boost::python;
        if (len(state) != 2)
        {
            PyErr_SetObject(PyExc_ValueError,
                            ("expected 2-item tuple in call to __setstate__; got %s"
                             % state).ptr()
                );
            throw_error_already_set();
        }

        p.set_allow_overlap(extract<bool>(state[0]));
        p.set_ignore_placement(extract<bool>(state[1]));
        //p.set_opacity(extract<float>(state[2]));

    }

};

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
        //.def_pickle(markers_symbolizer_pickle_suite())
        .add_property("filename",
                      &get_filename,
                      &set_filename)
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
        ;
}
