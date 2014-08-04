/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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
#ifndef GROUP_PROPERTIES_HPP
#define GROUP_PROPERTIES_HPP

// mapnik
#include <mapnik/group/group_layout.hpp>

// stl
#include <vector>
#include <memory>

namespace mapnik
{
struct group_rule;
using group_rule_ptr = std::shared_ptr<group_rule>;
using group_rules = std::vector<group_rule_ptr>;

/** Contains all group symbolizer properties related to building a group layout. */
struct group_symbolizer_properties
{
    inline group_symbolizer_properties() : rules_() { }
    /** Load all values from XML ptree. */
    //void from_xml(xml_node const &sym, fontset_map const & fontsets);
    /** Get the layout. */
    inline group_layout const& get_layout() const { return layout_; }
    /** Get the group rules. */
    inline group_rules const& get_rules() const { return rules_; }
    /** Set the layout. */
    inline void set_layout (group_layout && layout) { layout_ = std::move(layout); }
    /** Add add group rule. */
    inline void add_rule (group_rule_ptr rule) { rules_.push_back(rule); }

private:
    group_layout layout_;
    group_rules  rules_;
};

using group_symbolizer_properties_ptr = std::shared_ptr<group_symbolizer_properties>;

} //ns mapnik

#endif // GROUP_PROPERTIES_HPP
