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
#include <boost/version.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>
#include <boost/spirit/include/phoenix_core.hpp>

namespace mapnik { namespace json {

#if BOOST_VERSION >= 104700

template <typename Iterator>
geometry_parser<Iterator>::geometry_parser()
    : grammar_(new geometry_grammar<iterator_type>()) {}

template <typename Iterator>
geometry_parser<Iterator>::~geometry_parser() {}
#endif

template <typename Iterator>
bool geometry_parser<Iterator>::parse(iterator_type first, iterator_type last, boost::ptr_vector<mapnik::geometry_type>& path)
{
#if BOOST_VERSION >= 104700
    using namespace boost::spirit;
    return qi::phrase_parse(first, last, (*grammar_)(boost::phoenix::ref(path)), standard_wide::space);
#else
    std::ostringstream s;
    s << BOOST_VERSION/100000 << "." << BOOST_VERSION/100 % 1000  << "." << BOOST_VERSION % 100;
    throw std::runtime_error("mapnik::geometry_parser::parse() requires at least boost 1.47 while your build was compiled against boost " + s.str());
    return false;
#endif
}


bool from_geojson(std::string const& json, boost::ptr_vector<geometry_type> & paths)
{
#if BOOST_VERSION >= 104700
    geometry_parser<std::string::const_iterator> parser;
    std::string::const_iterator start = json.begin();
    std::string::const_iterator end = json.end();
    return parser.parse(start, end ,paths);
#else
    std::ostringstream s;
    s << BOOST_VERSION/100000 << "." << BOOST_VERSION/100 % 1000  << "." << BOOST_VERSION % 100;
    throw std::runtime_error("mapnik::json::from_geojson() requires at least boost 1.47 while your build was compiled against boost " + s.str());
    return false;
#endif
}

template class geometry_parser<std::string::const_iterator> ;
template class geometry_parser<boost::spirit::multi_pass<std::istreambuf_iterator<char> > >;

}}
