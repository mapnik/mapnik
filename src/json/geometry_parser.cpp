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
#include <mapnik/json/geometry_parser.hpp>
#include <mapnik/json/geometry_grammar.hpp>

// boost
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>
#include <boost/spirit/include/phoenix_core.hpp>

// stl
#include <stdexcept>

namespace mapnik { namespace json {

template <typename Iterator>
geometry_parser<Iterator>::geometry_parser()
    : grammar_(new geometry_grammar<iterator_type>()) {}

template <typename Iterator>
geometry_parser<Iterator>::~geometry_parser() {}

template <typename Iterator>
bool geometry_parser<Iterator>::parse(iterator_type first, iterator_type last, boost::ptr_vector<mapnik::geometry_type>& path)
{
    using namespace boost::spirit;
    standard_wide::space_type space;
    return qi::phrase_parse(first, last, (*grammar_)(boost::phoenix::ref(path)), space);
}


bool from_geojson(std::string const& json, boost::ptr_vector<geometry_type> & paths)
{
    geometry_parser<std::string::const_iterator> parser;
    std::string::const_iterator start = json.begin();
    std::string::const_iterator end = json.end();
    return parser.parse(start, end ,paths);
}

template class geometry_parser<std::string::const_iterator> ;
template class geometry_parser<boost::spirit::multi_pass<std::istreambuf_iterator<char> > >;

}}
