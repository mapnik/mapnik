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
#include <mapnik/json/feature_collection_parser.hpp>
#include <mapnik/json/feature_collection_grammar.hpp>

// boost
#include <boost/version.hpp>
#include <boost/spirit/include/qi.hpp>

namespace mapnik { namespace json {

feature_collection_parser::feature_collection_parser(mapnik::context_ptr const& ctx, mapnik::transcoder const& tr)
    : grammar_(new feature_collection_grammar<iterator_type,feature_type>(ctx,tr)) {}

feature_collection_parser::~feature_collection_parser() {}

bool feature_collection_parser::parse(std::string const& json, std::vector<mapnik::feature_ptr> & features)
{
    using namespace boost::spirit;
    iterator_type first = json.begin();
    iterator_type last =  json.end();
    return qi::phrase_parse(first, last, *grammar_, ascii::space, features);
}

}}

