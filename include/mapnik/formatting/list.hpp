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
#ifndef FORMATTING_LIST_HPP
#define FORMATTING_LIST_HPP

#include <mapnik/formatting/base.hpp>

namespace mapnik {
namespace formatting {
class list_node: public node {
public:
    list_node() : node(), children_() {}
    virtual void to_xml(boost::property_tree::ptree &xml) const;
    virtual void apply(char_properties const& p, Feature const& feature, processed_text &output) const;
    virtual void add_expressions(expression_set &output) const;

    void push_back(node_ptr n);
    void set_children(std::vector<node_ptr> const& children);
    std::vector<node_ptr> const& get_children() const;
    void clear();
protected:
    std::vector<node_ptr> children_;
};
} //ns formatting
} //ns mapnik

#endif // FORMATTING_LIST_HPP

