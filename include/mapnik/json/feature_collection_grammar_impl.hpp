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
#include <mapnik/json/error_handler.hpp>
#include <mapnik/json/feature_collection_grammar.hpp>

// spirit::qi
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace mapnik { namespace json {

template <typename Iterator, typename FeatureType>
feature_collection_grammar<Iterator,FeatureType>::feature_collection_grammar(mapnik::transcoder const& tr)
        : feature_collection_grammar::base_type(start,"start"),
          feature_g(tr),
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
        qi::_r2_type _r2;
        using phoenix::push_back;
        using phoenix::construct;
        using phoenix::new_;
        using phoenix::val;

        start = feature_collection(_r1) | feature_from_geometry(_r1, _val) | feature(_r1, _val)
            ;

        feature_collection = lit('{') >> (type | features(_r1) | feature_g.json_.key_value) % lit(',') >> lit('}')
            ;

        type = lit("\"type\"") >> lit(':') >> lit("\"FeatureCollection\"")
            ;

        features = lit("\"features\"")
            >> lit(':')
            >> lit('[')
            >> -(feature(_r1, _val) % lit(','))
            >> lit(']')
            ;

        feature = eps[_a = phoenix::construct<mapnik::feature_ptr>(new_<mapnik::feature_impl>(_r1, generate_id_()))]
            >> feature_g(*_a)[push_back(_r2,_a)]
            ;

        feature_from_geometry =
            eps[_a = phoenix::construct<mapnik::feature_ptr>(new_<mapnik::feature_impl>(_r1, generate_id_()))]
            >> geometry_g(extract_geometry_(*_a)) [push_back(_r2, _a)]
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

}}
