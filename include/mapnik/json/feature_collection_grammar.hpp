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
#include <mapnik/json/feature_grammar.hpp>

// spirit::qi
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant.hpp>

// stl
#include <iostream>

namespace mapnik { namespace json {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace standard_wide =  boost::spirit::standard_wide;
using standard_wide::space_type;

struct generate_id
{
    typedef int result_type;
    
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
    qi::grammar<Iterator, std::vector<feature_ptr>(), space_type>
{
    feature_collection_grammar(context_ptr const& ctx, mapnik::transcoder const& tr)
        : feature_collection_grammar::base_type(feature_collection,"feature-collection"),
          ctx_(ctx),
          feature_g(tr),
          generate_id_(1)
    {
        using qi::lit;
        using qi::eps;
        using qi::_a;
        using qi::_b;
        using qi::_val;
        using qi::_r1;
        using phoenix::push_back;
        using phoenix::construct;
        using phoenix::new_;
        using phoenix::val;

        feature_collection = lit('{') >> (type | features) % lit(",") >> lit('}')
            ;
        
        type = lit("\"type\"") > lit(":") > lit("\"FeatureCollection\"")
            ;

        features = lit("\"features\"")
            > lit(":")
            > lit('[')
            > -(feature(_val) % lit(','))
            > lit(']')
            ;
        
        feature = eps[_a = construct<feature_ptr>(new_<feature_impl>(ctx_,generate_id_()))]
            >> feature_g(*_a)[push_back(_r1,_a)]
            ;
        
        type.name("type");
        features.name("features");
        feature.name("feature");
        feature_g.name("feature-grammar");
        
        qi::on_error<qi::fail>
            (
                feature_collection
                , std::clog
                << phoenix::val("Error parsing GeoJSON ")
                << qi::_4                       
                << phoenix::val(" here: \"")
                << construct<std::string>(qi::_3, qi::_2) 
                << phoenix::val("\"")
                << std::endl
                );
    }
    
    context_ptr ctx_;
    qi::rule<Iterator, std::vector<feature_ptr>(), space_type> feature_collection; // START
    qi::rule<Iterator, space_type> type;
    qi::rule<Iterator, std::vector<feature_ptr>(), space_type> features;
    qi::rule<Iterator, qi::locals<feature_ptr,int>, void(std::vector<feature_ptr>&), space_type> feature;
    feature_grammar<Iterator,FeatureType> feature_g;
    boost::phoenix::function<generate_id> generate_id_;
};

}}

#endif // MAPNIK_FEATURE_COLLECTION_GRAMMAR_HPP
