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

#include <mapnik/json/parse_feature.hpp>
#include <mapnik/json/json_grammar_config.hpp>
#include <mapnik/json/feature_grammar_x3.hpp>

namespace mapnik { namespace json {

template <typename Iterator>
void parse_feature(Iterator start, Iterator end, feature_impl& feature, mapnik::transcoder const& tr)
{
    namespace x3 = boost::spirit::x3;
    using space_type = mapnik::json::grammar::space_type;
#if BOOST_VERSION >= 106700
    auto grammar = x3::with<mapnik::json::grammar::transcoder_tag>(tr)
        [x3::with<mapnik::json::grammar::feature_tag>(feature)
         [ mapnik::json::grammar::feature_rule ]];
#else
    auto grammar = x3::with<mapnik::json::grammar::transcoder_tag>(std::ref(tr))
        [x3::with<mapnik::json::grammar::feature_tag>(std::ref(feature))
         [ mapnik::json::grammar::feature_rule ]];
#endif
    if (!x3::phrase_parse(start, end, grammar, space_type()))
    {
        throw std::runtime_error("Can't parser GeoJSON Feature");
    }
}

template <typename Iterator>
void parse_geometry(Iterator start, Iterator end, feature_impl& feature)
{
    namespace x3 = boost::spirit::x3;
    using space_type = mapnik::json::grammar::space_type;
    auto grammar = mapnik::json::grammar::geometry_rule;
    if (!x3::phrase_parse(start, end, grammar, space_type(), feature.get_geometry()))
    {
        throw std::runtime_error("Can't parser GeoJSON Geometry");
    }
}

using iterator_type = mapnik::json::grammar::iterator_type;
template void parse_feature<iterator_type>(iterator_type,iterator_type, feature_impl& feature, mapnik::transcoder const& tr);
template void parse_geometry<iterator_type>(iterator_type,iterator_type, feature_impl& feature);

}}
