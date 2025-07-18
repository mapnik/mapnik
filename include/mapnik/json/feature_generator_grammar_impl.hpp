/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

namespace mapnik {
namespace json {

template<typename OutputIterator, typename FeatureType>
feature_generator_grammar<OutputIterator, FeatureType>::feature_generator_grammar()
    : feature_generator_grammar::base_type(feature)
{
    boost::spirit::karma::lit_type lit;
    boost::spirit::karma::long_long_type id;

    feature = lit("{\"type\":\"Feature\"")             //
              << lit(",\"id\":") << id                 //
              << lit(",\"geometry\":") << geometry     //
              << lit(",\"properties\":") << properties //
              << lit('}')                              //
      ;
}

} // namespace json
} // namespace mapnik
