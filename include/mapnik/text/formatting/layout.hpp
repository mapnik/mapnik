/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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
#include <optional>

namespace mapnik {
namespace formatting {

class MAPNIK_DECL layout_node : public node
{
  public:
    void to_xml(boost::property_tree::ptree& xml) const;
    static node_ptr from_xml(xml_node const& xml, fontset_map const& fontsets);
    virtual void apply(evaluated_format_properties_ptr const& p,
                       feature_impl const& feature,
                       attributes const& vars,
                       text_layout& output) const;
    virtual void add_expressions(expression_set& output) const;
    void set_child(node_ptr child);
    node_ptr get_child() const;
    //
    std::optional<symbolizer_base::value_type> dx;
    std::optional<symbolizer_base::value_type> dy;
    std::optional<symbolizer_base::value_type> halign;
    std::optional<symbolizer_base::value_type> valign;
    std::optional<symbolizer_base::value_type> jalign;
    std::optional<symbolizer_base::value_type> text_ratio;
    std::optional<symbolizer_base::value_type> wrap_width;
    std::optional<symbolizer_base::value_type> wrap_char;
    std::optional<symbolizer_base::value_type> wrap_before;
    std::optional<symbolizer_base::value_type> repeat_wrap_char;
    std::optional<symbolizer_base::value_type> rotate_displacement;
    std::optional<symbolizer_base::value_type> orientation;

  private:
    node_ptr child_;
};
} // namespace formatting
} // namespace mapnik

#endif // FORMATTING_OFFSET_HPP
