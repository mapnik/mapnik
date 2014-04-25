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

#ifndef FORMATTING_OFFSET_HPP
#define FORMATTING_OFFSET_HPP

#include <mapnik/text/formatting/base.hpp>
#include <mapnik/text/text_properties.hpp>

#include <boost/optional.hpp>

namespace mapnik {
namespace formatting {
class MAPNIK_DECL layout_node: public node {
public:
    void to_xml(boost::property_tree::ptree &xml) const;
    static node_ptr from_xml(xml_node const& xml);
    virtual void apply(char_properties_ptr p, feature_impl const& feature, text_layout &output) const;
    virtual void add_expressions(expression_set &output) const;
    void set_child(node_ptr child);
    node_ptr get_child() const;

    boost::optional<double> dx;
    boost::optional<double> dy;
    boost::optional<horizontal_alignment_e> halign;
    boost::optional<vertical_alignment_e> valign;
    boost::optional<justify_alignment_e> jalign;
    boost::optional<double> text_ratio;
    boost::optional<double> wrap_width;
    boost::optional<bool> wrap_before;
    boost::optional<bool> rotate_displacement;
    boost::optional<expression_ptr> orientation;

private:
    node_ptr child_;
};
} //ns formatting
} //ns mapnik

#endif // FORMATTING_OFFSET_HPP
