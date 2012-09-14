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

// boost
#include <boost/python.hpp>
#include <boost/python/implicit.hpp>
#include <boost/python/detail/api_placeholder.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

// mapnik
#include <mapnik/rule.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/expression_string.hpp>

using mapnik::rule;
using mapnik::expr_node;
using mapnik::expression_ptr;
using mapnik::Feature;
using mapnik::point_symbolizer;
using mapnik::line_symbolizer;
using mapnik::line_pattern_symbolizer;
using mapnik::polygon_symbolizer;
using mapnik::polygon_pattern_symbolizer;
using mapnik::raster_symbolizer;
using mapnik::shield_symbolizer;
using mapnik::text_symbolizer;
using mapnik::building_symbolizer;
using mapnik::markers_symbolizer;
using mapnik::symbolizer;
using mapnik::to_expression_string;

void export_rule()
{
    using namespace boost::python;
    implicitly_convertible<point_symbolizer,symbolizer>();
    implicitly_convertible<line_symbolizer,symbolizer>();
    implicitly_convertible<line_pattern_symbolizer,symbolizer>();
    implicitly_convertible<polygon_symbolizer,symbolizer>();
    implicitly_convertible<building_symbolizer,symbolizer>();
    implicitly_convertible<polygon_pattern_symbolizer,symbolizer>();
    implicitly_convertible<raster_symbolizer,symbolizer>();
    implicitly_convertible<shield_symbolizer,symbolizer>();
    implicitly_convertible<text_symbolizer,symbolizer>();
    implicitly_convertible<markers_symbolizer,symbolizer>();

    class_<rule::symbolizers>("Symbolizers",init<>("TODO"))
        .def(vector_indexing_suite<rule::symbolizers>())
        ;

    class_<rule>("Rule",init<>("default constructor"))
        .def(init<std::string const&,
             boost::python::optional<double,double> >())
        .add_property("name",make_function
                      (&rule::get_name,
                       return_value_policy<copy_const_reference>()),
                      &rule::set_name)
        .add_property("filter",make_function
                      (&rule::get_filter,return_value_policy<copy_const_reference>()),
                      &rule::set_filter)
        .add_property("min_scale",&rule::get_min_scale,&rule::set_min_scale)
        .add_property("max_scale",&rule::get_max_scale,&rule::set_max_scale)
        .def("set_else",&rule::set_else)
        .def("has_else",&rule::has_else_filter)
        .def("set_also",&rule::set_also)
        .def("has_also",&rule::has_also_filter)
        .def("active",&rule::active)
        .add_property("symbols",make_function
                      (&rule::get_symbolizers,return_value_policy<reference_existing_object>()))
        ;
}

