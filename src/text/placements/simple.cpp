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

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/text/placements/simple.hpp>
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/xml_node.hpp>

// boost
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

#include <boost/property_tree/ptree.hpp>

namespace mapnik
{

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
using phoenix::push_back;
using phoenix::ref;

bool text_placement_info_simple::next()
{
    while (1) {
        if (state > 0)
        {
            if (state > parent_->text_sizes_.size()) return false;
            properties.format->text_size = parent_->text_sizes_[state-1];
        }
        if (!next_position_only()) {
            state++;
            position_state = 0;
        } else {
            break;
        }
    }
    return true;
}

bool text_placement_info_simple::next_position_only()
{
    pixel_position const& pdisp = parent_->defaults.displacement;
    pixel_position &displacement = properties.displacement;
    if (position_state >= parent_->direction_.size()) return false;
    directions_t dir = parent_->direction_[position_state];
    switch (dir) {
    case EXACT_POSITION:
        displacement = pdisp;
        break;
    case NORTH:
        displacement.set(0, -abs(pdisp.y));
        break;
    case EAST:
        displacement.set(abs(pdisp.x), 0);
        break;
    case SOUTH:
        displacement.set(0, abs(pdisp.y));
        break;
    case WEST:
        displacement.set(-abs(pdisp.x), 0);
        break;
    case NORTHEAST:
        displacement.set(abs(pdisp.x), -abs(pdisp.y));
        break;
    case SOUTHEAST:
        displacement.set(abs(pdisp.x), abs(pdisp.y));
        break;
    case NORTHWEST:
        displacement.set(-abs(pdisp.x), -abs(pdisp.y));
        break;
    case SOUTHWEST:
        displacement.set(-abs(pdisp.x), abs(pdisp.y));
        break;
    default:
        MAPNIK_LOG_WARN(text_placements) << "Unknown placement";
    }
    position_state++;
    return true;
}

text_placement_info_ptr text_placements_simple::get_placement_info(
    double scale_factor) const
{
    return std::make_shared<text_placement_info_simple>(this, scale_factor);
}

/** Position string: [POS][SIZE]
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
    boost::spirit::ascii::space_type space;
    qi::_1_type _1;
    qi::float_type float_;

    std::string::iterator first = positions.begin(),  last = positions.end();
    qi::phrase_parse(first, last,
                     (direction_name[push_back(phoenix::ref(direction_), _1)] % ',') >> *(',' >> float_[push_back(phoenix::ref(text_sizes_), _1)]),
                     space
        );
    if (first != last)
    {
        MAPNIK_LOG_WARN(text_placements) << "Could not parse text_placement_simple placement string ('" << positions << "')";
    }
    if (direction_.size() == 0)
    {
        MAPNIK_LOG_WARN(text_placements) << "text_placements_simple with no valid placements! ('"<< positions<<"')";
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

std::string text_placements_simple::get_positions()
{
    return positions_; //TODO: Build string from data in direction_ and text_sizes_
}

text_placements_ptr text_placements_simple::from_xml(xml_node const &xml, fontset_map const & fontsets)
{
    text_placements_ptr ptr = std::make_shared<text_placements_simple>(
        xml.get_attr<std::string>("placements", "X"));
    ptr->defaults.from_xml(xml, fontsets);
    return ptr;
}

} //ns mapnik
