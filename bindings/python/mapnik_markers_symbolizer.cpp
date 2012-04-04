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
//$Id$

#include <boost/python.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/markers_symbolizer.hpp>
#include <mapnik/parse_path.hpp>
#include "mapnik_svg.hpp"

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
    }

};


void export_markers_symbolizer()
{
    using namespace boost::python;
    
    class_<markers_symbolizer>("MarkersSymbolizer",
                             init<>("Default Markers Symbolizer - blue arrow"))
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
                      "Set/get the text opacity")
        .add_property("ignore_placement",
                      &markers_symbolizer::get_ignore_placement,
                      &markers_symbolizer::set_ignore_placement)
        .add_property("transform",
                      &mapnik::get_svg_transform<markers_symbolizer>,
                      &mapnik::set_svg_transform<markers_symbolizer>)
        .add_property("width",
                      &markers_symbolizer::get_width,
                      &markers_symbolizer::set_width,
                      "Set/get the marker width")
        .add_property("height",
                      &markers_symbolizer::get_height,
                      &markers_symbolizer::set_height,
                      "Set/get the marker height")
        ;
}
