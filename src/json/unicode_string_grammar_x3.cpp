/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2016 Artem Pavlenko
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

#include <mapnik/json/unicode_string_grammar_x3_def.hpp>
#include <mapnik/json/json_grammar_config.hpp>
#include <mapnik/json/extract_bounding_boxes_x3_config.hpp>
#include <mapnik/json/json_value.hpp>
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#pragma GCC diagnostic pop

namespace mapnik { namespace json { namespace grammar {

BOOST_SPIRIT_INSTANTIATE(unicode_string_grammar_type, iterator_type, phrase_parse_context_type);
BOOST_SPIRIT_INSTANTIATE(unicode_string_grammar_type, iterator_type, context_type);
BOOST_SPIRIT_INSTANTIATE(unicode_string_grammar_type, iterator_type, feature_context_type);
BOOST_SPIRIT_INSTANTIATE(unicode_string_grammar_type, iterator_type, feature_context_const_type);
BOOST_SPIRIT_INSTANTIATE(unicode_string_grammar_type, iterator_type, extract_bounding_boxes_context_type);
BOOST_SPIRIT_INSTANTIATE(unicode_string_grammar_type, iterator_type, extract_bounding_boxes_reverse_context_type);
BOOST_SPIRIT_INSTANTIATE(unicode_string_grammar_type, iterator_type, extract_bounding_boxes_context_type_f);
BOOST_SPIRIT_INSTANTIATE(unicode_string_grammar_type, iterator_type, extract_bounding_boxes_reverse_context_type_f);

BOOST_SPIRIT_INSTANTIATE_UNUSED(unicode_string_grammar_type, iterator_type, phrase_parse_context_type);
BOOST_SPIRIT_INSTANTIATE_UNUSED(unicode_string_grammar_type, iterator_type, feature_context_type);
BOOST_SPIRIT_INSTANTIATE_UNUSED(unicode_string_grammar_type, iterator_type, feature_context_const_type);
BOOST_SPIRIT_INSTANTIATE_UNUSED(unicode_string_grammar_type, iterator_type, extract_bounding_boxes_context_type);
BOOST_SPIRIT_INSTANTIATE_UNUSED(unicode_string_grammar_type, iterator_type, extract_bounding_boxes_reverse_context_type);
BOOST_SPIRIT_INSTANTIATE_UNUSED(unicode_string_grammar_type, iterator_type, extract_bounding_boxes_context_type_f);
BOOST_SPIRIT_INSTANTIATE_UNUSED(unicode_string_grammar_type, iterator_type, extract_bounding_boxes_reverse_context_type_f);

}}}

template bool mapnik::json::grammar::parse_rule<char const*, boost::spirit::x3::context<boost::spirit::x3::skipper_tag, boost::spirit::x3::char_class<boost::spirit::char_encoding::standard, boost::spirit::x3::space_tag> const, boost::spirit::x3::unused_type>, boost::fusion::iterator_range<boost::fusion::std_tuple_iterator<std::tuple<std::basic_string<char, std::char_traits<char>, std::allocator<char> >, mapnik::json::json_value>, 0>, boost::fusion::std_tuple_iterator<std::tuple<std::basic_string<char, std::char_traits<char>, std::allocator<char> >, mapnik::json::json_value>, 1> > >(boost::spirit::x3::rule<mapnik::json::grammar::unicode_string_tag, std::basic_string<char, std::char_traits<char>, std::allocator<char> >, false>, char const*&, char const* const&, boost::spirit::x3::context<boost::spirit::x3::skipper_tag, boost::spirit::x3::char_class<boost::spirit::char_encoding::standard, boost::spirit::x3::space_tag> const, boost::spirit::x3::unused_type> const&, boost::fusion::iterator_range<boost::fusion::std_tuple_iterator<std::tuple<std::basic_string<char, std::char_traits<char>, std::allocator<char> >, mapnik::json::json_value>, 0>, boost::fusion::std_tuple_iterator<std::tuple<std::basic_string<char, std::char_traits<char>, std::allocator<char> >, mapnik::json::json_value>, 1> >&);
