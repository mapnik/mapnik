/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#include <mapnik/json/feature_generator_grammar.hpp>

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/fusion/include/at.hpp>

namespace mapnik { namespace json {

template <typename OutputIterator, typename FeatureType>
feature_generator_grammar<OutputIterator, FeatureType>::feature_generator_grammar()
  : feature_generator_grammar::base_type(feature)
{
    boost::spirit::karma::lit_type lit;
    boost::spirit::karma::uint_type uint_;
    boost::spirit::karma::_val_type _val;
    boost::spirit::karma::_1_type _1;

    feature = lit("{\"type\":\"Feature\",\"id\":")
        << uint_[_1 = id_(_val)]
        << lit(",\"geometry\":") << geometry
        << lit(",\"properties\":") << properties
        << lit('}')
        ;
}

}}
