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

#ifndef MAPNIK_TEXT_PLACEMENTS_SIMPLE_HPP
#define MAPNIK_TEXT_PLACEMENTS_SIMPLE_HPP

// mapnik
#include <mapnik/text/placements/base.hpp>

namespace mapnik {

class text_placement_info_simple;
class feature_impl;
struct attribute;

// Automatically generates placement options from a user selected list of directions and text sizes.
class MAPNIK_DECL text_placements_simple : public text_placements
{
  public:
    text_placements_simple(symbolizer_base::value_type const& positions);
    text_placements_simple(symbolizer_base::value_type const& positions,
                           std::vector<directions_e>&& direction,
                           std::vector<int>&& text_sizes);
    text_placement_info_ptr
      get_placement_info(double _scale_factor, feature_impl const& feature, attributes const& vars) const;
    std::string get_positions() const;
    static text_placements_ptr from_xml(xml_node const& xml, fontset_map const& fontsets, bool is_shield);
    std::vector<directions_e> direction_;
    std::vector<int> text_sizes_;

  private:
    symbolizer_base::value_type positions_;
    friend class text_placement_info_simple;
};

// Simple placement strategy.
// See parent class for documentation of each function.
class MAPNIK_DECL text_placement_info_simple : public text_placement_info
{
  public:
    text_placement_info_simple(text_placements_simple const* parent,
                               std::string const& evaluated_positions,
                               double _scale_factor);
    bool next() const;

  protected:
    bool next_position_only() const;
    mutable unsigned state;
    mutable unsigned position_state;
    mutable std::vector<directions_e> direction_;
    mutable std::vector<int> text_sizes_;
    text_placements_simple const* parent_;
};

} // namespace mapnik

#endif // MAPNIK_TEXT_PLACEMENTS_SIMPLE_HPP
