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
#ifndef FORMATTING_BASE_HPP
#define FORMATTING_BASE_HPP

// mapnik
#include <mapnik/expression.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/text/evaluated_format_properties_ptr.hpp>

// boost
#include <boost/property_tree/ptree_fwd.hpp>

// stl
#include <memory>
#include <map>

namespace mapnik {

class text_layout;
class feature_impl;
class xml_node;
class font_set;

namespace formatting {

class node;
using node_ptr = std::shared_ptr<node>;
using fontset_map = std::map<std::string, font_set>;

class MAPNIK_DECL node
{
  public:
    virtual ~node() {}
    virtual void to_xml(boost::property_tree::ptree& xml) const = 0;
    static node_ptr from_xml(xml_node const& xml, fontset_map const& fontsets);
    virtual void apply(evaluated_format_properties_ptr const& p,
                       feature_impl const& feature,
                       attributes const& vars,
                       text_layout& output) const = 0;
    virtual void add_expressions(expression_set& output) const = 0;
};
} // namespace formatting
} // namespace mapnik
#endif // FORMATTING_BASE_HPP
