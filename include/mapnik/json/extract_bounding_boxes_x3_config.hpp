/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_JSON_EXTRACT_BOUNDING_BOXES_CONFIG_HPP
#define MAPNIK_JSON_EXTRACT_BOUNDING_BOXES_CONFIG_HPP

#include <mapnik/json/json_grammar_config.hpp>
#include <mapnik/geometry/box2d.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
#pragma GCC diagnostic pop

//auto feature_collection_impl = x3::with<mapnik::json::grammar::bracket_tag>(std::ref(bracket_counter))
// [x3::with<mapnik::json::grammar::keys_tag>(std::ref(keys))
//       [x3::with<mapnik::json::grammar::feature_callback_tag>(std::ref(callback))
//        [mapnik::json::grammar::feature_collection]
//           ]];

namespace mapnik { namespace json {

template <typename Iterator, typename Boxes>
struct extract_positions
{
    using boxes_type = Boxes;
    using box_type = typename boxes_type::value_type::first_type;

    extract_positions(Iterator start, Boxes & boxes)
        : start_(start),
          boxes_(boxes) {}

    template <typename T>
    void operator() (T const& val) const
    {
        auto const& r = std::get<0>(val);
        auto const& b = std::get<1>(val);
        if (b.valid())
        {
            auto offset = std::distance(start_, r.begin());
            auto size = std::distance(r.begin(), r.end());
            boxes_.emplace_back(std::make_pair(box_type(b.minx(), b.miny(), b.maxx(), b.maxy()), std::make_pair(offset, size)));
        }
    }
    Iterator start_;
    Boxes & boxes_;
};

using box_type = mapnik::box2d<double>;
using boxes_type = std::vector<std::pair<box_type, std::pair<std::uint64_t, std::uint64_t>>>;
using callback_type = extract_positions<grammar::iterator_type, boxes_type>;

using box_type_f = mapnik::box2d<float>;
using boxes_type_f = std::vector<std::pair<box_type_f, std::pair<std::uint64_t, std::uint64_t>>>;
using callback_type_f = extract_positions<grammar::iterator_type, boxes_type_f>;


namespace grammar {

struct bracket_tag;
struct feature_callback_tag;

namespace x3 = boost::spirit::x3;
using space_type = x3::standard::space_type;

using phrase_parse_context_type = x3::phrase_parse_context<space_type>::type;

using context_type = x3::with_context<keys_tag, std::reference_wrapper<keys_map> const,
                                      phrase_parse_context_type>::type;

using extract_bounding_boxes_context_type =
    x3::with_context<bracket_tag, std::reference_wrapper<std::size_t> const,
                     x3::with_context<feature_callback_tag, std::reference_wrapper<callback_type> const,
                                      context_type>::type>::type;

using extract_bounding_boxes_reverse_context_type =
    x3::with_context<keys_tag, std::reference_wrapper<keys_map> const,
                     x3::with_context<feature_callback_tag, std::reference_wrapper<callback_type> const,
                                      x3::with_context<bracket_tag, std::reference_wrapper<std::size_t> const,
                                                       phrase_parse_context_type>::type>::type>::type;


using extract_bounding_boxes_context_type_f =
    x3::with_context<bracket_tag, std::reference_wrapper<std::size_t> const,
                     x3::with_context<feature_callback_tag, std::reference_wrapper<callback_type_f> const,
                                      context_type>::type>::type;

using extract_bounding_boxes_reverse_context_type_f =
    x3::with_context<keys_tag, std::reference_wrapper<keys_map> const,
                     x3::with_context<feature_callback_tag, std::reference_wrapper<callback_type_f> const,
                                      x3::with_context<bracket_tag, std::reference_wrapper<std::size_t> const,
                                                       phrase_parse_context_type>::type>::type>::type;

}}}

#endif // MAPNIK_JSON_EXTRACT_BOUNDING_BOXES_CONFIG_HPP
