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
    feature_collection_grammar( generic_json<Iterator> & json, context_ptr const& ctx, mapnik::transcoder const& tr)
        : feature_collection_grammar::base_type(start,"start"),
          feature_g(json,tr),
          ctx_(ctx),
          generate_id_(1)
    {
        qi::lit_type lit;
        qi::eps_type eps;
        qi::_4_type _4;
        qi::_3_type _2;
        qi::_2_type _3;
        qi::_a_type _a;
        qi::_val_type _val;
        qi::_r1_type _r1;
        using phoenix::push_back;
        using phoenix::construct;
        using phoenix::new_;
        using phoenix::val;

        start = feature_collection | feature_from_geometry(_val) | feature(_val)
            ;

        feature_collection = lit('{') >> (type | features) % lit(',') >> lit('}')
            ;

        type = lit("\"type\"") >> lit(':') >> lit("\"FeatureCollection\"")
            ;

        features = lit("\"features\"")
            >> lit(':')
            >> lit('[')
            >> -(feature(_val) % lit(','))
            >> lit(']')
            ;

        feature = eps[_a = phoenix::construct<mapnik::feature_ptr>(new_<mapnik::feature_impl>(ctx_,generate_id_()))]
            >> feature_g(*_a)[push_back(_r1,_a)]
            ;

        feature_from_geometry =
            eps[_a = phoenix::construct<mapnik::feature_ptr>(new_<mapnik::feature_impl>(ctx_,generate_id_()))]
            >> geometry_g(extract_geometry_(*_a)) [push_back(_r1, _a)]
            ;

        start.name("start");
        type.name("type");
        features.name("features");
        feature.name("feature");
        feature_from_geometry.name("feature-from-geometry");
        feature_g.name("feature-grammar");
        geometry_g.name("geometry-grammar");

        qi::on_error<qi::fail>
            (
                feature_collection
                , std::clog
                << phoenix::val("Error parsing GeoJSON ")
                << _4
                << phoenix::val(" here: \"")
                << construct<std::string>(_3, _2)
                << phoenix::val('\"')
                << std::endl
                );
    }

    feature_grammar<Iterator,FeatureType> feature_g;
    geometry_grammar<Iterator> geometry_g;
    phoenix::function<extract_geometry> extract_geometry_;
    context_ptr ctx_;
    qi::rule<Iterator, std::vector<feature_ptr>(), space_type> start; // START
    qi::rule<Iterator, std::vector<feature_ptr>(), space_type> feature_collection;
    qi::rule<Iterator, space_type> type;
    qi::rule<Iterator, std::vector<feature_ptr>(), space_type> features;
    qi::rule<Iterator, qi::locals<feature_ptr,int>, void(std::vector<feature_ptr>&), space_type> feature;
    qi::rule<Iterator, qi::locals<feature_ptr,int>, void(std::vector<feature_ptr>&), space_type> feature_from_geometry;
    boost::phoenix::function<generate_id> generate_id_;
};

}}

#endif // MAPNIK_FEATURE_COLLECTION_GRAMMAR_HPP
