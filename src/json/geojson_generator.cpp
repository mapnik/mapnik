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

// boost
#include <mapnik/feature.hpp>
#include <mapnik/json/geojson_generator.hpp>
#include <mapnik/json/feature_generator_grammar.hpp>
#include <mapnik/json/geometry_generator_grammar.hpp>

#include <boost/spirit/include/karma.hpp>

namespace  mapnik { namespace json {

feature_generator::feature_generator()
    : grammar_(new feature_generator_grammar<sink_type>()) {}

feature_generator::~feature_generator() {}

bool feature_generator::generate(std::string & geojson, mapnik::feature_impl const& f)
{
    sink_type sink(geojson);
    return karma::generate(sink, *grammar_,f);
}

geometry_generator::geometry_generator()
    : grammar_(new multi_geometry_generator_grammar<sink_type>()) {}

geometry_generator::~geometry_generator() {}

bool geometry_generator::generate(std::string & geojson, mapnik::geometry_container const& g)
{
    sink_type sink(geojson);
    return karma::generate(sink, *grammar_,g);
}

}}
