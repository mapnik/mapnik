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
// mapnik
#include <mapnik/formatting/text.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/text_properties.hpp>
#include <mapnik/processed_text.hpp>
#include <mapnik/xml_node.hpp>

namespace mapnik
{
namespace formatting
{
using boost::property_tree::ptree;


void text_node::to_xml(ptree &xml) const
{
    ptree &new_node = xml.push_back(ptree::value_type(
                                        "<xmltext>", ptree()))->second;
    new_node.put_value(to_expression_string(*text_));
}

node_ptr text_node::from_xml(xml_node const& xml)
{
    return boost::make_shared<text_node>(xml.get_value<expression_ptr>());
}

void text_node::apply(char_properties const& p, Feature const& feature, processed_text &output) const
{
    UnicodeString text_str = boost::apply_visitor(evaluate<Feature,value_type>(feature), *text_).to_unicode();
    if (p.text_transform == UPPERCASE)
    {
        text_str = text_str.toUpper();
    }
    else if (p.text_transform == LOWERCASE)
    {
        text_str = text_str.toLower();
    }
    else if (p.text_transform == CAPITALIZE)
    {
        text_str = text_str.toTitle(NULL);
    }
    if (text_str.length() > 0) {
        output.push_back(p, text_str);
    }
}


void text_node::add_expressions(expression_set &output) const
{
    if (text_) output.insert(text_);
}


void text_node::set_text(expression_ptr text)
{
    text_ = text;
}


expression_ptr text_node::get_text() const
{
    return text_;
}

} //ns formatting
} //ns mapnik
