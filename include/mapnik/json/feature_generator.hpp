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

#ifndef MAPNIK_JSON_FEATURE_GENERATOR_HPP
#define MAPNIK_JSON_FEATURE_GENERATOR_HPP

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/json/feature_generator_grammar.hpp>

// boost
#include <boost/spirit/include/karma.hpp>

namespace mapnik { namespace json {

inline bool to_geojson(std::string & json, mapnik::feature_impl const& feature)
{
    using sink_type = std::back_insert_iterator<std::string>;
    static const mapnik::json::feature_generator_grammar<sink_type, mapnik::feature_impl> grammar;
    sink_type sink(json);
    return boost::spirit::karma::generate(sink, grammar, feature);
}

}}

#endif // MAPNIK_JSON_FEATURE_GENERATOR_HPP
