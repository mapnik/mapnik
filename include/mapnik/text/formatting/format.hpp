/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

// mapnik
#include <mapnik/text/formatting/base.hpp>
#include <mapnik/text/text_properties.hpp>

// boost
#include <boost/property_tree/ptree_fwd.hpp>

namespace mapnik {

class feature_impl;

namespace formatting {

class MAPNIK_DECL format_node: public node
{
public:
    void to_xml(boost::property_tree::ptree & xml) const;
    static node_ptr from_xml(xml_node const& xml, fontset_map const& fontsets);
    virtual void apply(evaluated_format_properties_ptr const& p, feature_impl const& feature, attributes const& vars, text_layout & output) const;
    virtual void add_expressions(expression_set & output) const;

    void set_child(node_ptr child);
    node_ptr get_child() const;

    boost::optional<std::string> face_name;
    boost::optional<font_set> fontset;
    boost::optional<symbolizer_base::value_type> text_size;
    boost::optional<symbolizer_base::value_type> character_spacing;
    boost::optional<symbolizer_base::value_type> line_spacing;
    boost::optional<symbolizer_base::value_type> text_opacity;
    boost::optional<symbolizer_base::value_type> wrap_before;
    boost::optional<symbolizer_base::value_type> repeat_wrap_char;
    boost::optional<symbolizer_base::value_type> text_transform;
    boost::optional<symbolizer_base::value_type> fill;
    boost::optional<symbolizer_base::value_type> halo_fill;
    boost::optional<symbolizer_base::value_type> halo_radius;
    boost::optional<symbolizer_base::value_type> ff_settings;

private:
    node_ptr child_;
};
} //ns formatting
} //ns mapnik

#endif // FORMATTING_FORMAT_HPP
