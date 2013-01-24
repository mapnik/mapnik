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

#include <mapnik/line_pattern_symbolizer.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/image_util.hpp>
#include "mapnik_svg.hpp"

using mapnik::line_pattern_symbolizer;
using mapnik::path_processor_type;
using mapnik::path_expression_ptr;
using mapnik::guess_type;
using mapnik::parse_path;


namespace {
using namespace boost::python;

std::string get_filename(line_pattern_symbolizer const& t)
{
    return path_processor_type::to_string(*t.get_filename());
}

void set_filename(line_pattern_symbolizer & t, std::string const& file_expr)
{
    t.set_filename(parse_path(file_expr));
}

}

void export_line_pattern_symbolizer()
{
    using namespace boost::python;

    class_<line_pattern_symbolizer>("LinePatternSymbolizer",
                                    init<path_expression_ptr>
                                    ("<image file expression>"))
        .add_property("transform",
                      mapnik::get_svg_transform<line_pattern_symbolizer>,
                      mapnik::set_svg_transform<line_pattern_symbolizer>)
        .add_property("filename",
                      &get_filename,
                      &set_filename)
        .add_property("comp_op",
                      &line_pattern_symbolizer::comp_op,
                      &line_pattern_symbolizer::set_comp_op,
                      "Set/get the comp-op")
        .add_property("clip",
                      &line_pattern_symbolizer::clip,
                      &line_pattern_symbolizer::set_clip,
                      "Set/get the line pattern geometry's clipping status")
        .add_property("smooth",
                      &line_pattern_symbolizer::smooth,
                      &line_pattern_symbolizer::set_smooth,
                      "smooth value (0..1.0)")
        ;
}
