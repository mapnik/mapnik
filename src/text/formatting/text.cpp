/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <mapnik/text/formatting/text.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/xml_node.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/debug.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/property_tree/ptree.hpp>
#pragma GCC diagnostic pop

namespace mapnik
{
namespace formatting
{

using boost::property_tree::ptree;

void text_node::to_xml(ptree & xml) const
{
    ptree & new_node = xml.push_back(ptree::value_type("<xmltext>", ptree()))->second;
    new_node.put_value(to_expression_string(*text_));
}

node_ptr text_node::from_xml(xml_node const& xml, fontset_map const&)
{
    return std::make_shared<text_node>(xml.get_value<expression_ptr>());
}

void text_node::apply(evaluated_format_properties_ptr const& p, feature_impl const& feature, attributes const& vars, text_layout &output) const
{
    mapnik::value_unicode_string text_str = util::apply_visitor(evaluate<feature_impl,value_type,attributes>(feature,vars), *text_).to_unicode();
    switch (p->text_transform)
    {
    case UPPERCASE:
        text_str.toUpper();
        break;
    case LOWERCASE:
        text_str.toLower();
        break;
    case REVERSE:
        text_str.reverse();
        break;
    case CAPITALIZE:
#if !UCONFIG_NO_BREAK_ITERATION
        // note: requires BreakIterator support in ICU which is optional
        text_str.toTitle(nullptr);
#else
        MAPNIK_LOG_DEBUG(text_node) << "text capitalize (toTitle) disabled because ICU was built without UCONFIG_NO_BREAK_ITERATION";
#endif
        break;
    default:
        break;
    }
    if (text_str.length() > 0) {
        output.add_text(text_str, p);
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
