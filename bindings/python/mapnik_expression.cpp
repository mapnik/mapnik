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
#include <boost/variant.hpp>

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/value.hpp>


using mapnik::Feature;
using mapnik::expression_ptr;
using mapnik::parse_expression;
using mapnik::to_expression_string;
using mapnik::path_expression_ptr;


// expression
expression_ptr parse_expression_(std::string const& wkt)
{
    return parse_expression(wkt,"utf8");
}

std::string expression_to_string_(mapnik::expr_node const& expr)
{
    return mapnik::to_expression_string(expr);
}

mapnik::value expression_evaluate_(mapnik::expr_node const& expr, mapnik::Feature const& f)
{
    // will be auto-converted to proper python type by `mapnik_value_to_python`
    return boost::apply_visitor(mapnik::evaluate<mapnik::Feature,mapnik::value>(f),expr);
}

// path expression
path_expression_ptr parse_path_(std::string const& path)
{
    return mapnik::parse_path(path);
}

std::string path_to_string_(mapnik::path_expression const& expr)
{
    return mapnik::path_processor_type::to_string(expr);
}

std::string path_evaluate_(mapnik::path_expression const& expr, mapnik::Feature const& f)
{
    return mapnik::path_processor_type::evaluate(expr, f);
}

void export_expression()
{
    using namespace boost::python;
    class_<mapnik::expr_node ,boost::noncopyable>("Expression",
                                                  "TODO"
                                                  "",no_init)
        .def("evaluate", &expression_evaluate_)
        .def("__str__",&expression_to_string_);
    ;

    def("Expression",&parse_expression_,(arg("expr")),"Expression string");

    class_<mapnik::path_expression ,boost::noncopyable>("PathExpression",
                                                        "TODO"
                                                        "",no_init)
        .def("evaluate", &path_evaluate_) // note: "pass" is a reserved word in Python
        .def("__str__",&path_to_string_);
    ;

    def("PathExpression",&parse_path_,(arg("expr")),"PathExpression string");
}
