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
#include "mapnik_enumeration.hpp"
#include <mapnik/graphics.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/point_symbolizer.hpp>
#include <mapnik/parse_path.hpp>
#include "mapnik_svg.hpp"

using namespace mapnik;
using mapnik::point_symbolizer;
using mapnik::symbolizer_with_image;
using mapnik::path_processor_type;
using mapnik::parse_path;


namespace {
using namespace boost::python;

std::string get_filename(point_symbolizer const& t)
{
    return path_processor_type::to_string(*t.get_filename());
}

void set_filename(point_symbolizer & t, std::string const& file_expr)
{
    t.set_filename(parse_path(file_expr));
}

}

void export_point_symbolizer()
{
    using namespace boost::python;

    enumeration_<point_placement_e>("point_placement")
        .value("CENTROID",CENTROID_POINT_PLACEMENT)
        .value("INTERIOR",INTERIOR_POINT_PLACEMENT)
        ;

    class_<point_symbolizer>("PointSymbolizer",
                             init<>("Default Point Symbolizer - 4x4 black square"))
        .def (init<mapnik::path_expression_ptr>("<path expression ptr>"))
        .add_property("filename",
                      &get_filename,
                      &set_filename)
        .add_property("allow_overlap",
                      &point_symbolizer::get_allow_overlap,
                      &point_symbolizer::set_allow_overlap)
        .add_property("opacity",
                      &point_symbolizer::get_opacity,
                      &point_symbolizer::set_opacity)
        .add_property("ignore_placement",
                      &point_symbolizer::get_ignore_placement,
                      &point_symbolizer::set_ignore_placement)
        .add_property("placement",
                      &point_symbolizer::get_point_placement,
                      &point_symbolizer::set_point_placement,
                      "Set/get the placement of the point")
        .add_property("transform",
                      mapnik::get_svg_transform<point_symbolizer>,
                      mapnik::set_svg_transform<point_symbolizer>)
        .add_property("comp_op",
                      &point_symbolizer::comp_op,
                      &point_symbolizer::set_comp_op,
                      "Set/get the comp-op")
        ;
}
