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

#ifndef MAPNIK_FEATURE_COLLECTION_GRAMMAR_HPP
#define MAPNIK_FEATURE_COLLECTION_GRAMMAR_HPP

// mapnik
#include <mapnik/unicode.hpp>
#include <mapnik/json/geometry_grammar.hpp>
#include <mapnik/json/feature_grammar.hpp>
#include <mapnik/feature.hpp>

// spirit::qi
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace mapnik { namespace json {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace standard_wide =  boost::spirit::standard_wide;
using standard_wide::space_type;

struct generate_id
{
    using result_type = int;

    generate_id(int start)
        : id_(start) {}

    int operator() () const
    {
        return id_++;
    }
    mutable int id_;
};

template <typename Iterator, typename FeatureType>
struct feature_collection_grammar :
    qi::grammar<Iterator, std::vector<feature_ptr>(context_ptr const&), space_type>
{
    feature_collection_grammar(mapnik::transcoder const& tr);
    feature_grammar<Iterator,FeatureType> feature_g;
    geometry_grammar<Iterator> geometry_g;
    phoenix::function<extract_geometry> extract_geometry_;
    qi::rule<Iterator, std::vector<feature_ptr>(context_ptr const&), space_type> start; // START
    qi::rule<Iterator, std::vector<feature_ptr>(context_ptr const&), space_type> feature_collection;
    qi::rule<Iterator, space_type> type;
    qi::rule<Iterator, std::vector<feature_ptr>(context_ptr const&), space_type> features;
    qi::rule<Iterator, qi::locals<feature_ptr,int>, void(context_ptr const& ctx, std::vector<feature_ptr>&), space_type> feature;
    qi::rule<Iterator, qi::locals<feature_ptr,int>, void(context_ptr const& ctx, std::vector<feature_ptr>&), space_type> feature_from_geometry;
    boost::phoenix::function<generate_id> generate_id_;
};

}}

#endif // MAPNIK_FEATURE_COLLECTION_GRAMMAR_HPP
