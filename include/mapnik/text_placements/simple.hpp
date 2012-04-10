/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
#include <mapnik/text_placements/base.hpp>

namespace mapnik {

class text_placement_info_simple;

typedef enum {
    NORTH,
    EAST,
    SOUTH,
    WEST,
    NORTHEAST,
    SOUTHEAST,
    NORTHWEST,
    SOUTHWEST,
    EXACT_POSITION
} directions_t;


/** Automatically generates placement options from a user selected list of directions and text sizes. */
class text_placements_simple: public text_placements
{
public:
    text_placements_simple();
    text_placements_simple(std::string positions);
    text_placement_info_ptr get_placement_info(double scale_factor) const;
    void set_positions(std::string positions);
    std::string get_positions();
    static text_placements_ptr from_xml(xml_node const &xml, fontset_map const & fontsets);
private:
    std::string positions_;
    std::vector<directions_t> direction_;
    std::vector<int> text_sizes_;
    friend class text_placement_info_simple;
};

/** Simple placement strategy.
 * See parent class for documentation of each function. */
class text_placement_info_simple : public text_placement_info
{
public:
    text_placement_info_simple(text_placements_simple const* parent,
                               double scale_factor)
        : text_placement_info(parent, scale_factor),
          state(0), position_state(0), parent_(parent)
    {
    }
    bool next();
protected:
    bool next_position_only();
    unsigned state;
    unsigned position_state;
    text_placements_simple const* parent_;
};

} //namespace

#endif // MAPNIK_TEXT_PLACEMENTS_SIMPLE_HPP
