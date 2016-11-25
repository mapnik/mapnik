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

namespace mapnik { namespace json { namespace grammar {

BOOST_SPIRIT_INSTANTIATE(unicode_string_grammar_type, iterator_type, phrase_parse_context_type);
BOOST_SPIRIT_INSTANTIATE(unicode_string_grammar_type, iterator_type, context_type);
BOOST_SPIRIT_INSTANTIATE(unicode_string_grammar_type, iterator_type, geometry_context_type);
BOOST_SPIRIT_INSTANTIATE(unicode_string_grammar_type, iterator_type, feature_context_type);
BOOST_SPIRIT_INSTANTIATE(unicode_string_grammar_type, iterator_type, feature_context_const_type);
BOOST_SPIRIT_INSTANTIATE_UNUSED(unicode_string_grammar_type, iterator_type, geometry_context_type);
BOOST_SPIRIT_INSTANTIATE_UNUSED(unicode_string_grammar_type, iterator_type, feature_context_type);
BOOST_SPIRIT_INSTANTIATE_UNUSED(unicode_string_grammar_type, iterator_type, feature_context_const_type);

BOOST_SPIRIT_INSTANTIATE(unicode_string_grammar_type, iterator_type, extract_bounding_boxes_context_type);
BOOST_SPIRIT_INSTANTIATE_UNUSED(unicode_string_grammar_type, iterator_type, extract_bounding_boxes_context_type);
}}}


template bool mapnik::json::grammar::parse_rule<char const*, boost::spirit::x3::context<mapnik::json::grammar::keys_tag, std::reference_wrapper<boost::bimaps::bimap<boost::bimaps::unordered_set_of<std::basic_string<char, std::char_traits<char>, std::allocator<char> >, boost::hash<std::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::equal_to<std::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, boost::bimaps::set_of<int, std::less<int> >, mpl_::na, mpl_::na, mpl_::na> > const, boost::spirit::x3::context<mapnik::json::grammar::feature_callback_tag, std::reference_wrapper<mapnik::json::extract_positions<char const*, std::vector<std::pair<mapnik::box2d<float>, std::pair<unsigned long, unsigned long> >, std::allocator<std::pair<mapnik::box2d<float>, std::pair<unsigned long, unsigned long> > > > > > const, boost::spirit::x3::context<mapnik::json::grammar::bracket_tag, std::reference_wrapper<unsigned long> const, boost::spirit::x3::context<boost::spirit::x3::skipper_tag, boost::spirit::x3::char_class<boost::spirit::char_encoding::standard, boost::spirit::x3::space_tag> const, boost::spirit::x3::unused_type> > > >, boost::spirit::x3::unused_type const>(boost::spirit::x3::rule<mapnik::json::grammar::unicode_string_tag, std::basic_string<char, std::char_traits<char>, std::allocator<char> >, false>, char const*&, char const* const&, boost::spirit::x3::context<mapnik::json::grammar::keys_tag, std::reference_wrapper<boost::bimaps::bimap<boost::bimaps::unordered_set_of<std::basic_string<char, std::char_traits<char>, std::allocator<char> >, boost::hash<std::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::equal_to<std::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, boost::bimaps::set_of<int, std::less<int> >, mpl_::na, mpl_::na, mpl_::na> > const, boost::spirit::x3::context<mapnik::json::grammar::feature_callback_tag, std::reference_wrapper<mapnik::json::extract_positions<char const*, std::vector<std::pair<mapnik::box2d<float>, std::pair<unsigned long, unsigned long> >, std::allocator<std::pair<mapnik::box2d<float>, std::pair<unsigned long, unsigned long> > > > > > const, boost::spirit::x3::context<mapnik::json::grammar::bracket_tag, std::reference_wrapper<unsigned long> const, boost::spirit::x3::context<boost::spirit::x3::skipper_tag, boost::spirit::x3::char_class<boost::spirit::char_encoding::standard, boost::spirit::x3::space_tag> const, boost::spirit::x3::unused_type> > > > const&, boost::spirit::x3::unused_type const&);


template bool mapnik::json::grammar::parse_rule<char const*, boost::spirit::x3::context<mapnik::json::grammar::keys_tag, std::reference_wrapper<boost::bimaps::bimap<boost::bimaps::unordered_set_of<std::basic_string<char, std::char_traits<char>, std::allocator<char> >, boost::hash<std::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::equal_to<std::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, boost::bimaps::set_of<int, std::less<int> >, mpl_::na, mpl_::na, mpl_::na> > const, boost::spirit::x3::context<mapnik::json::grammar::feature_callback_tag, std::reference_wrapper<mapnik::json::extract_positions<char const*, std::vector<std::pair<mapnik::box2d<float>, std::pair<unsigned long, unsigned long> >, std::allocator<std::pair<mapnik::box2d<float>, std::pair<unsigned long, unsigned long> > > > > > const, boost::spirit::x3::context<mapnik::json::grammar::bracket_tag, std::reference_wrapper<unsigned long> const, boost::spirit::x3::context<boost::spirit::x3::skipper_tag, boost::spirit::x3::char_class<boost::spirit::char_encoding::standard, boost::spirit::x3::space_tag> const, boost::spirit::x3::unused_type> > > >, std::basic_string<char, std::char_traits<char>, std::allocator<char> > >(boost::spirit::x3::rule<mapnik::json::grammar::unicode_string_tag, std::basic_string<char, std::char_traits<char>, std::allocator<char> >, false>, char const*&, char const* const&, boost::spirit::x3::context<mapnik::json::grammar::keys_tag, std::reference_wrapper<boost::bimaps::bimap<boost::bimaps::unordered_set_of<std::basic_string<char, std::char_traits<char>, std::allocator<char> >, boost::hash<std::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::equal_to<std::basic_string<char, std::char_traits<char>, std::allocator<char> > > >, boost::bimaps::set_of<int, std::less<int> >, mpl_::na, mpl_::na, mpl_::na> > const, boost::spirit::x3::context<mapnik::json::grammar::feature_callback_tag, std::reference_wrapper<mapnik::json::extract_positions<char const*, std::vector<std::pair<mapnik::box2d<float>, std::pair<unsigned long, unsigned long> >, std::allocator<std::pair<mapnik::box2d<float>, std::pair<unsigned long, unsigned long> > > > > > const, boost::spirit::x3::context<mapnik::json::grammar::bracket_tag, std::reference_wrapper<unsigned long> const, boost::spirit::x3::context<boost::spirit::x3::skipper_tag, boost::spirit::x3::char_class<boost::spirit::char_encoding::standard, boost::spirit::x3::space_tag> const, boost::spirit::x3::unused_type> > > > const&, std::basic_string<char, std::char_traits<char>, std::allocator<char> >&);
