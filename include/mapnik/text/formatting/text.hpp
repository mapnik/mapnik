/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#ifndef FORMATTING_TEXT_HPP
#define FORMATTING_TEXT_HPP

// mapnik
#include <mapnik/text/formatting/base.hpp>

// boost
#include <boost/property_tree/ptree_fwd.hpp>

namespace mapnik {

class feature_impl;

namespace formatting {
class MAPNIK_DECL text_node : public node
{
  public:
    text_node(expression_ptr text)
        : node()
        , text_(text)
    {}
    text_node(std::string text)
        : node()
        , text_(parse_expression(text))
    {}
    void to_xml(boost::property_tree::ptree& xml) const;
    static node_ptr from_xml(xml_node const& xml, fontset_map const& fontsets);
    virtual void apply(evaluated_format_properties_ptr const& p,
                       feature_impl const& feature,
                       attributes const& vars,
                       text_layout& output) const;
    virtual void add_expressions(expression_set& output) const;

    void set_text(expression_ptr text);
    expression_ptr get_text() const;

  private:
    expression_ptr text_;
};
} // namespace formatting
} // namespace mapnik

#endif // FORMATTING_TEXT_HPP
