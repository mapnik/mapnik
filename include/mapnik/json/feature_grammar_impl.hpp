/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <mapnik/feature.hpp>
#include <mapnik/json/feature_grammar.hpp>

namespace mapnik { namespace json {

template <typename Iterator, typename FeatureType, typename ErrorHandler>
feature_grammar<Iterator,FeatureType,ErrorHandler>::feature_grammar(mapnik::transcoder const& tr)
    : feature_grammar::base_type(start,"feature"),
      json_(),
      put_property_(put_property(tr))
{
    qi::lit_type lit;
    qi::long_long_type long_long;
    qi::double_type double_;
    qi::_1_type _1;
    qi::_2_type _2;
    qi::_3_type _3;
    qi::_4_type _4;
    qi::_a_type _a;
    qi::_r1_type _r1;
    qi::eps_type eps;
    qi::char_type char_;
    using qi::fail;
    using qi::on_error;
    using phoenix::new_;
    using phoenix::construct;

    // geojson types
    feature_type = lit("\"type\"") > lit(':') > lit("\"Feature\"")
        ;

    start = feature(_r1);

    feature = eps[_a = false] > lit('{') >
        (feature_type[_a = true]
         |
         (lit("\"geometry\"") > lit(':') > geometry_grammar_[set_geometry(_r1, _1)])
         |
         properties(_r1)
         |
         json_.key_value) % lit(',')
        > eps(_a) > lit('}')
        ;

    properties = lit("\"properties\"")
        > lit(':') > ((lit('{') > -attributes(_r1) > lit('}')) | lit("null"))
        ;

    attributes = (json_.string_ [_a = _1] > lit(':') > json_.value [put_property_(_r1,_a,_1)]) % lit(',')
        ;

    feature.name("Feature");
    feature_type.name("type");
    properties.name("properties");
    attributes.name("Attributes");
    on_error<fail>(feature, error_handler(_1, _2, _3, _4));

}

}}
