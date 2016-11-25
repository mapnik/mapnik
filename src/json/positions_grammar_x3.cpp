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

#include <mapnik/json/positions_grammar_x3_def.hpp>
#include <mapnik/json/json_grammar_config.hpp>
#include <mapnik/json/extract_bounding_boxes_x3_config.hpp>

namespace mapnik { namespace json { namespace grammar {

BOOST_SPIRIT_INSTANTIATE(positions_grammar_type, iterator_type, phrase_parse_context_type);
BOOST_SPIRIT_INSTANTIATE(positions_grammar_type, iterator_type, context_type);
BOOST_SPIRIT_INSTANTIATE(positions_grammar_type, iterator_type, geometry_context_type);
BOOST_SPIRIT_INSTANTIATE(positions_grammar_type, iterator_type, feature_context_type);
BOOST_SPIRIT_INSTANTIATE(positions_grammar_type, iterator_type, feature_context_const_type);

BOOST_SPIRIT_INSTANTIATE(positions_grammar_type, iterator_type, extract_bounding_boxes_context_type);
BOOST_SPIRIT_INSTANTIATE_UNUSED(positions_grammar_type, iterator_type, extract_bounding_boxes_context_type);
}}}


template bool mapnik::json::grammar::parse_rule<char const*, boost::spirit::x3::context<mapnik::json::grammar::keys_tag, std::__1::reference_wrapper<boost::bimaps::bimap<boost::bimaps::unordered_set_of<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >, boost::hash<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > >, std::__1::equal_to<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > > >, boost::bimaps::set_of<int, std::__1::less<int> >, mpl_::na, mpl_::na, mpl_::na> > const, boost::spirit::x3::context<mapnik::json::grammar::feature_callback_tag, std::__1::reference_wrapper<mapnik::json::extract_positions<char const*, std::__1::vector<std::__1::pair<mapnik::box2d<float>, std::__1::pair<unsigned long, unsigned long> >, std::__1::allocator<std::__1::pair<mapnik::box2d<float>, std::__1::pair<unsigned long, unsigned long> > > > > > const, boost::spirit::x3::context<mapnik::json::grammar::bracket_tag, std::__1::reference_wrapper<unsigned long> const, boost::spirit::x3::context<boost::spirit::x3::skipper_tag, boost::spirit::x3::char_class<boost::spirit::char_encoding::standard, boost::spirit::x3::space_tag> const, boost::spirit::x3::unused_type> > > >, mapbox::util::variant<mapnik::geometry::point<double>, std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > >, std::__1::vector<std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > >, std::__1::allocator<std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > > > >, std::__1::vector<std::__1::vector<std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > >, std::__1::allocator<std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > > > >, std::__1::allocator<std::__1::vector<std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > >, std::__1::allocator<std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > > > > > > > >(boost::spirit::x3::rule<mapnik::json::grammar::positions_tag, mapbox::util::variant<mapnik::geometry::point<double>, std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > >, std::__1::vector<std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > >, std::__1::allocator<std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > > > >, std::__1::vector<std::__1::vector<std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > >, std::__1::allocator<std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > > > >, std::__1::allocator<std::__1::vector<std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > >, std::__1::allocator<std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > > > > > > >, false>, char const*&, char const* const&, boost::spirit::x3::context<mapnik::json::grammar::keys_tag, std::__1::reference_wrapper<boost::bimaps::bimap<boost::bimaps::unordered_set_of<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >, boost::hash<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > >, std::__1::equal_to<std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > > >, boost::bimaps::set_of<int, std::__1::less<int> >, mpl_::na, mpl_::na, mpl_::na> > const, boost::spirit::x3::context<mapnik::json::grammar::feature_callback_tag, std::__1::reference_wrapper<mapnik::json::extract_positions<char const*, std::__1::vector<std::__1::pair<mapnik::box2d<float>, std::__1::pair<unsigned long, unsigned long> >, std::__1::allocator<std::__1::pair<mapnik::box2d<float>, std::__1::pair<unsigned long, unsigned long> > > > > > const, boost::spirit::x3::context<mapnik::json::grammar::bracket_tag, std::__1::reference_wrapper<unsigned long> const, boost::spirit::x3::context<boost::spirit::x3::skipper_tag, boost::spirit::x3::char_class<boost::spirit::char_encoding::standard, boost::spirit::x3::space_tag> const, boost::spirit::x3::unused_type> > > > const&, mapbox::util::variant<mapnik::geometry::point<double>, std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > >, std::__1::vector<std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > >, std::__1::allocator<std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > > > >, std::__1::vector<std::__1::vector<std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > >, std::__1::allocator<std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > > > >, std::__1::allocator<std::__1::vector<std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > >, std::__1::allocator<std::__1::vector<mapnik::geometry::point<double>, std::__1::allocator<mapnik::geometry::point<double> > > > > > > >&);
