/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

#ifndef MAPNIK_FEATURE_GRAMMAR_HPP
#define MAPNIK_FEATURE_GRAMMAR_HPP

// mapnik
#include <mapnik/json/geometry_grammar.hpp>
#include <mapnik/value.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/geometry_container.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/value.hpp>
#include <mapnik/json/generic_json.hpp>
#include <mapnik/json/value_converters.hpp>
// spirit::qi
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/support_line_pos_iterator.hpp>

namespace mapnik { namespace json {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace fusion = boost::fusion;
namespace standard_wide =  boost::spirit::standard_wide;
using standard_wide::space_type;

class attribute_value_visitor
    : public mapnik::util::static_visitor<mapnik::value>
{
public:
    attribute_value_visitor(mapnik::transcoder const& tr)
        : tr_(tr) {}

    mapnik::value operator()(std::string const& val) const
    {
        return mapnik::value(tr_.transcode(val.c_str()));
    }

    template <typename T>
    mapnik::value operator()(T const& val) const
    {
        return mapnik::value(val);
    }

    mapnik::transcoder const& tr_;
};

struct put_property
{
    using result_type = void;
    explicit put_property(mapnik::transcoder const& tr)
        : tr_(tr) {}
    template <typename T0,typename T1, typename T2>
    result_type operator() (T0 & feature, T1 const& key, T2 && val) const
    {
        feature.put_new(key, mapnik::util::apply_visitor(attribute_value_visitor(tr_),val));
    }
    mapnik::transcoder const& tr_;
};

struct extract_geometry
{
    using result_type =  mapnik::geometry_container&;
    template <typename T>
    result_type operator() (T & feature) const
    {
        return feature.paths();
    }
};

template <typename Iterator, typename FeatureType, typename ErrorHandler = error_handler<Iterator> >
struct feature_grammar :
        qi::grammar<Iterator, void(FeatureType&),
                    space_type>
{
    feature_grammar(mapnik::transcoder const& tr);

    // start
    // generic JSON
    generic_json<Iterator> json_;

    // geoJSON
    qi::rule<Iterator,void(FeatureType&),space_type> feature; // START
    qi::rule<Iterator,space_type> feature_type;

    qi::rule<Iterator,void(FeatureType &),space_type> properties;
    qi::rule<Iterator,qi::locals<std::string>, void(FeatureType &),space_type> attributes;
    qi::rule<Iterator, json_value(), space_type> attribute_value;

    phoenix::function<put_property> put_property_;
    phoenix::function<extract_geometry> extract_geometry_;
    // error handler
    boost::phoenix::function<ErrorHandler> const error_handler;
    // geometry
    geometry_grammar<Iterator> geometry_grammar_;
};

}}

#endif // MAPNIK_FEATURE_GRAMMAR_HPP
