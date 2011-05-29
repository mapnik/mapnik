/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Hermann Kraus
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

#include <mapnik/text_placements.hpp>
#include <mapnik/text_placements_simple.hpp>

#include <boost/tuple/tuple.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

namespace mapnik {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
using boost::spirit::ascii::space;
using phoenix::push_back;
using phoenix::ref;
using qi::_1;


/************************************************************************/

text_placement_info::text_placement_info(text_placements const* parent):
    displacement(parent->displacement_),
    text_size(parent->text_size_), halign(parent->halign_), jalign(parent->jalign_),
    valign(parent->valign_)
{

}

bool text_placement_info_dummy::next()
{
    if (state) return false;
    state++;
    return true;
}

bool text_placement_info_dummy::next_position_only()
{
    if (position_state) return false;
    position_state++;
    return true;
}

text_placement_info_ptr text_placements_dummy::get_placement_info() const
{
    return text_placement_info_ptr(new text_placement_info_dummy(this));
}


/************************************************************************/

bool text_placement_info_simple::next()
{
    position_state = 0;
    if (state == 0) {
        text_size = parent_->text_size_;
    } else {
        if (state > parent_->text_sizes_.size()) return false;
        text_size = parent_->text_sizes_[state-1];
    }
    state++;
    return true;
}

bool text_placement_info_simple::next_position_only()
{
    if (position_state >= parent_->direction_.size()) return false;
    directions_t dir = parent_->direction_[position_state];
    switch (dir) {
    case EXACT_POSITION:
        displacement = parent_->displacement_;
        break;
    case NORTH:
        displacement = boost::make_tuple(0, -abs(parent_->displacement_.get<1>()));
        break;
    case EAST:
        displacement = boost::make_tuple(abs(parent_->displacement_.get<0>()), 0);
        break;
    case SOUTH:
        displacement = boost::make_tuple(0, abs(parent_->displacement_.get<1>()));
        break;
    case WEST:
        displacement = boost::make_tuple(-abs(parent_->displacement_.get<0>()), 0);
        break;
    case NORTHEAST:
        displacement = boost::make_tuple(
                     abs(parent_->displacement_.get<0>()),
                    -abs(parent_->displacement_.get<1>()));
    case SOUTHEAST:
        displacement = boost::make_tuple(
                     abs(parent_->displacement_.get<0>()),
                     abs(parent_->displacement_.get<1>()));
    case NORTHWEST:
        displacement = boost::make_tuple(
                    -abs(parent_->displacement_.get<0>()),
                    -abs(parent_->displacement_.get<1>()));
    case SOUTHWEST:
        displacement = boost::make_tuple(
                    -abs(parent_->displacement_.get<0>()),
                     abs(parent_->displacement_.get<1>()));
        break;
    default:
        std::cerr << "WARNING: Unknown placement\n";
    }
    position_state++;
    return true;
}


text_placement_info_ptr text_placements_simple::get_placement_info() const
{
    return text_placement_info_ptr(new text_placement_info_simple(this));
}

/** Positiion string: [POS][SIZE]
  * [POS] is any combination of
  * N, E, S, W, NE, SE, NW, SW, X (exact position) (separated by commas)
  * [SIZE] is a list of font sizes, separated by commas. The first font size
  * is always the one given in the TextSymbolizer's parameters.
  * First all directions are tried, then font size is reduced
  * and all directions are tried again. The process ends when a placement is
  * found or the last fontsize is tried without success.
  * Example: N,S,15,10,8 (tries placement above, then below and if
  *    that fails it tries the additional font sizes 15, 10 and 8.
  */
void text_placements_simple::set_positions(std::string positions)
{
    positions_ = positions;
    struct direction_name_ : qi::symbols<char, directions_t>
    {
        direction_name_()
        {
            add
                ("N" , NORTH)
                ("E" , EAST)
                ("S" , SOUTH)
                ("W" , WEST)
                ("NE", NORTHEAST)
                ("SE", SOUTHEAST)
                ("NW", NORTHWEST)
                ("SW", SOUTHWEST)
                ("X" , EXACT_POSITION)
            ;
        }

    } direction_name;

    std::string::iterator first = positions.begin(),  last = positions.end();
    qi::phrase_parse(first, last,
        (direction_name[push_back(ref(direction_), _1)] % ',') >> *(',' >> qi::int_[push_back(ref(text_sizes_), _1)]),
        space
    );
    if (first != last) {
        std::cerr << "WARNING: Could not parse text_placement_simple placement string ('" << positions << "').\n";
    }
    if (direction_.size() == 0) {
        std::cerr << "WARNING: text_placements_simple with no valid placments! ('"<< positions<<"')\n";
    }
}

text_placements_simple::text_placements_simple()
{
    set_positions("X");
}

text_placements_simple::text_placements_simple(std::string positions)
{
    set_positions(positions);
}
} //namespace
