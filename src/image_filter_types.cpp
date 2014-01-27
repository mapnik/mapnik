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
#include <mapnik/image_filter_types.hpp>
#include <mapnik/image_filter_grammar.hpp> // image_filter_grammar

// boost
#include <boost/spirit/include/karma.hpp>
#include <boost/variant.hpp>

// stl
#include <vector>

namespace mapnik {

namespace filter {

bool generate_image_filters(std::back_insert_iterator<std::string>& sink, std::vector<filter_type> const& filters)
{
    boost::spirit::karma::stream_type stream;
    using boost::spirit::karma::generate;
    bool r = generate(sink, stream % ' ', filters);
    return r;
}

bool parse_image_filters(std::string const& filters, std::vector<filter_type>& image_filters)
{
    std::string::const_iterator itr = filters.begin();
    std::string::const_iterator end = filters.end();
    mapnik::image_filter_grammar<std::string::const_iterator,
                                 std::vector<mapnik::filter::filter_type> > filter_grammar;
    boost::spirit::qi::ascii::space_type space;
    bool r = boost::spirit::qi::phrase_parse(itr,end,
                                             filter_grammar,
                                             space,
                                             image_filters);
    return r && itr==end;
}

}}
