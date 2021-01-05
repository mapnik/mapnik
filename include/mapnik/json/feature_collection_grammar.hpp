/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

struct default_feature_callback
{
    default_feature_callback(std::vector<feature_ptr> & features)
        : features_(features) {}
    void operator() (feature_ptr const& feature)
    {
        features_.push_back(feature);
    }
    std::vector<feature_ptr> & features_;
};

struct apply_feature_callback
{
    using result_type = void;
    template <typename Callback, typename Feature>
    void operator() (Callback & callback, Feature const& feature) const
    {
        callback(feature);
    }
};

template <typename Iterator, typename FeatureType, typename FeatureCallback = default_feature_callback, typename ErrorHandler = error_handler<Iterator> >
struct feature_collection_grammar :
        qi::grammar<Iterator, void(context_ptr const&, std::size_t&, FeatureCallback &), space_type>
{
    feature_collection_grammar(mapnik::transcoder const& tr);
    // grammars
    feature_grammar<Iterator, FeatureType, ErrorHandler> feature_g;
    // rules
    qi::rule<Iterator, void(context_ptr const&, std::size_t&, FeatureCallback&), space_type> start; // START
    qi::rule<Iterator, void(context_ptr const&, std::size_t&, FeatureCallback&), space_type> feature_collection;
    qi::rule<Iterator, space_type> type;
    qi::rule<Iterator, void(context_ptr const&, std::size_t&, FeatureCallback&), space_type> features;
    qi::rule<Iterator, qi::locals<feature_ptr,int>, void(context_ptr const& ctx, std::size_t, FeatureCallback&), space_type> feature;
    // phoenix functions
    phoenix::function<apply_feature_callback> on_feature;
    // error handler
    boost::phoenix::function<ErrorHandler> const error_handler;
};

template <typename Iterator, typename FeatureType, typename FeatureCallback = default_feature_callback, typename ErrorHandler = error_handler<Iterator> >
struct feature_grammar_callback :
        qi::grammar<Iterator, void(context_ptr const&, std::size_t&, FeatureCallback &), space_type>
{
    feature_grammar_callback(mapnik::transcoder const& tr);
    // grammars
    feature_grammar<Iterator, FeatureType> feature_g;
    geometry_grammar<Iterator> geometry_g;
    // rules
    qi::rule<Iterator, void(context_ptr const&, std::size_t&, FeatureCallback&), space_type> start; // START
    qi::rule<Iterator, qi::locals<feature_ptr,int>, void(context_ptr const& ctx, std::size_t, FeatureCallback&), space_type> feature;
    qi::rule<Iterator, qi::locals<feature_ptr,int>, void(context_ptr const& ctx, std::size_t, FeatureCallback&), space_type> feature_from_geometry;
    // phoenix functions
    phoenix::function<json::set_geometry_impl> set_geometry;
    phoenix::function<apply_feature_callback> on_feature;
    // error handler
    boost::phoenix::function<ErrorHandler> const error_handler;
};


}}

#endif // MAPNIK_FEATURE_COLLECTION_GRAMMAR_HPP
