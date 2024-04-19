/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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
#include <mapnik/value.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/expression_string.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/property_tree/ptree.hpp>
MAPNIK_DISABLE_WARNING_POP

#include <cmath>

namespace mapnik {

namespace x3 = boost::spirit::x3;

struct direction_name : x3::symbols<directions_e>
{
    direction_name()
    {
        add("N", NORTH)("E", EAST)("S", SOUTH)("W", WEST)("NE", NORTHEAST)("SE", SOUTHEAST)("NW", NORTHWEST)(
          "SW",
          SOUTHWEST)("X", EXACT_POSITION)("C", CENTER);
    }
} names;

// Position string: [POS][SIZE]
// [POS] is any combination of
// N, E, S, W, NE, SE, NW, SW, X (exact position) (separated by commas)
// [SIZE] is a list of font sizes, separated by commas. The first font size
// is always the one given in the TextSymbolizer's parameters.
// First all directions are tried, then font size is reduced
// and all directions are tried again. The process ends when a placement is
// found or the last fontsize is tried without success.
// Example: N,S,15,10,8 (tries placement above, then below and if
//    that fails it tries the additional font sizes 15, 10 and 8.

bool parse_positions(std::string const& evaluated_positions,
                     std::vector<directions_e>& direction,
                     std::vector<int>& text_sizes)
{
    auto push_back_direction = [&](auto const& ctx) {
        direction.push_back(_attr(ctx));
    };
    auto push_back_size = [&](auto const& ctx) {
        text_sizes.push_back(_attr(ctx));
    };
    auto const first = evaluated_positions.begin();
    auto const last = evaluated_positions.end();
    bool r = x3::phrase_parse(first,
                              last,
                              (names[push_back_direction] % ',') >> *(',' >> x3::float_[push_back_size]),
                              x3::space);
    return (r && first != last);
}

text_placement_info_simple::text_placement_info_simple(text_placements_simple const* parent,
                                                       std::string const& evaluated_positions,
                                                       double scale_factor_)
    : text_placement_info(parent, scale_factor_)
    , state(0)
    , position_state(0)
    , direction_(parent->direction_)
    , text_sizes_(parent->text_sizes_)
    , parent_(parent)
{
    if (direction_.empty() && !parse_positions(evaluated_positions, direction_, text_sizes_))
    {
        MAPNIK_LOG_ERROR(text_placements)
          << "Could not parse text_placement_simple placement string ('" << evaluated_positions << "')";
        if (direction_.size() == 0)
        {
            MAPNIK_LOG_ERROR(text_placements)
              << "text_placements_simple with no valid placements! ('" << evaluated_positions << "')";
        }
    }
}

bool text_placement_info_simple::next() const
{
    while (true)
    {
        if (state > 0)
        {
            if (state > text_sizes_.size())
                return false;
            properties.format_defaults.text_size = value_double(text_sizes_[state - 1]);
        }
        if (!next_position_only())
        {
            ++state;
            position_state = 0;
        }
        else
            break;
    }
    return true;
}

bool text_placement_info_simple::next_position_only() const
{
    if (position_state >= direction_.size())
        return false;
    properties.layout_defaults.dir = direction_[position_state];
    ++position_state;
    return true;
}

text_placement_info_ptr text_placements_simple::get_placement_info(double scale_factor_,
                                                                   feature_impl const& feature,
                                                                   attributes const& vars) const
{
    std::string evaluated_positions = util::apply_visitor(extract_value<std::string>(feature, vars), positions_);
    return std::make_shared<text_placement_info_simple>(this, evaluated_positions, scale_factor_);
}

text_placements_simple::text_placements_simple(symbolizer_base::value_type const& positions)
    : direction_()
    , text_sizes_()
    , positions_(positions)
{}

text_placements_simple::text_placements_simple(symbolizer_base::value_type const& positions,
                                               std::vector<directions_e>&& direction,
                                               std::vector<int>&& text_sizes)
    : direction_(direction)
    , text_sizes_(text_sizes)
    , positions_(positions)
{}

namespace detail {
struct serialize_positions
{
    serialize_positions() {}

    std::string operator()(expression_ptr const& expr) const
    {
        if (expr)
            return to_expression_string(*expr);
        return "";
    }

    std::string operator()(std::string const val) const { return val; }

    template<typename T>
    std::string operator()(T const&) const
    {
        return "";
    }
};
} // namespace detail

std::string text_placements_simple::get_positions() const
{
    return util::apply_visitor(detail::serialize_positions(), positions_);
}

text_placements_ptr text_placements_simple::from_xml(xml_node const& xml, fontset_map const& fontsets, bool is_shield)
{
    // TODO - handle X cleaner
    std::string placements_string = xml.get_attr<std::string>("placements", "X");
    // like set_property_from_xml in properties_util.hpp
    if (!placements_string.empty())
    {
        if (placements_string == "X")
        {
            text_placements_ptr ptr = std::make_shared<text_placements_simple>(placements_string);
            ptr->defaults.from_xml(xml, fontsets, is_shield);
            return ptr;
        }
        else
        {
            try
            {
                // we don't use parse_expression(placements_string) directly here to benefit from the cache in the
                // xml_node
                const auto val = xml.get_opt_attr<expression_ptr>("placements");
                if (val.has_value())
                {
                    text_placements_ptr ptr = std::make_shared<text_placements_simple>(*val);
                    ptr->defaults.from_xml(xml, fontsets, is_shield);
                    return ptr;
                }
            }
            catch (std::exception const& ex)
            {
                // otherwise ensure it is valid
                std::vector<directions_e> direction;
                std::vector<int> text_sizes;
                if (!parse_positions(placements_string, direction, text_sizes))
                {
                    MAPNIK_LOG_ERROR(text_placements)
                      << "Could not parse text_placement_simple placement string ('" << placements_string << "')";
                    if (direction.size() == 0)
                    {
                        MAPNIK_LOG_ERROR(text_placements)
                          << "text_placements_simple with no valid placements! ('" << placements_string << "')";
                    }
                    return text_placements_ptr();
                }
                else
                {
                    text_placements_ptr ptr = std::make_shared<text_placements_simple>(placements_string,
                                                                                       std::move(direction),
                                                                                       std::move(text_sizes));
                    ptr->defaults.from_xml(xml, fontsets, is_shield);
                    return ptr;
                }
            }
            text_placements_ptr ptr = std::make_shared<text_placements_simple>(placements_string);
            ptr->defaults.from_xml(xml, fontsets, is_shield);
            return ptr;
        }
    }
    return text_placements_ptr();
}

} // namespace mapnik
