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

#ifndef MAPNIK_GEOJSON_GENERATOR_HPP
#define MAPNIK_GEOJSON_GENERATOR_HPP

#include <mapnik/config.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/noncopyable.hpp>


#include <string>
#include <iterator>

namespace mapnik { namespace json {

template <typename OutputIterator> struct feature_generator_grammar;
template <typename OutputIterator> struct multi_geometry_generator_grammar;

class MAPNIK_DECL feature_generator : private mapnik::noncopyable
{
    typedef std::back_insert_iterator<std::string> sink_type;
public:
    feature_generator();
    ~feature_generator();
    bool generate(std::string & geojson, mapnik::feature_impl const& f);
private:
    const std::unique_ptr<feature_generator_grammar<sink_type> > grammar_;
};

class MAPNIK_DECL geometry_generator : private mapnik::noncopyable
{
    typedef std::back_insert_iterator<std::string> sink_type;
public:
    geometry_generator();
    ~geometry_generator();
    bool generate(std::string & geojson, mapnik::geometry_container const& g);
private:
    const std::unique_ptr<multi_geometry_generator_grammar<sink_type> > grammar_;
};

}}

#endif // MAPNIK_GEOJSON_GENERATOR_HPP
