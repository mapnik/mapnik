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

#ifndef FORMATTING_FORMAT_HPP
#define FORMATTING_FORMAT_HPP

#include <mapnik/formatting/base.hpp>
#include <mapnik/text_properties.hpp>

namespace mapnik {
namespace formatting {
class format_node: public node {
public:
    void to_xml(boost::property_tree::ptree &xml) const;
    static node_ptr from_xml(xml_node const& xml);
    virtual void apply(char_properties const& p, Feature const& feature, processed_text &output) const;
    virtual void add_expressions(expression_set &output) const;

    void set_child(node_ptr child);
    node_ptr get_child() const;

    boost::optional<std::string> face_name;
    boost::optional<unsigned> text_size;
    boost::optional<unsigned> character_spacing;
    boost::optional<unsigned> line_spacing;
    boost::optional<double> text_opacity;
    boost::optional<bool> wrap_before;
    boost::optional<unsigned> wrap_char;
    boost::optional<text_transform_e> text_transform;
    boost::optional<color> fill;
    boost::optional<color> halo_fill;
    boost::optional<double> halo_radius;

private:
    node_ptr child_;
};
} //ns formatting
} //ns mapnik

#endif // FORMATTING_FORMAT_HPP
