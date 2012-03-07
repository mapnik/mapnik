/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

#ifndef FORMATTING_EXPRESSION_HPP
#define FORMATTING_EXPRESSION_HPP

#include <mapnik/formatting/base.hpp>
#include <mapnik/expression.hpp>

namespace mapnik {
namespace formatting {
class expression_format: public node {
public:
    void to_xml(boost::property_tree::ptree &xml) const;
    static node_ptr from_xml(xml_node const& xml);
    virtual void apply(char_properties const& p, Feature const& feature, processed_text &output) const;
    virtual void add_expressions(expression_set &output) const;

    void set_child(node_ptr child);
    node_ptr get_child() const;

    expression_ptr face_name;
    expression_ptr text_size;
    expression_ptr character_spacing;
    expression_ptr line_spacing;
    expression_ptr text_opacity;
    expression_ptr wrap_before;
    expression_ptr wrap_char;
    expression_ptr fill;
    expression_ptr halo_fill;
    expression_ptr halo_radius;

private:
    node_ptr child_;
    static expression_ptr get_expression(xml_node const& xml, std::string name);
};
} //ns formatting
} //ns mapnik
#endif // FORMATTING_EXPRESSION_HPP
