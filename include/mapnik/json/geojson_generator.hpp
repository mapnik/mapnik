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

#include <mapnik/feature.hpp>
#include <boost/scoped_ptr.hpp>
#include <string>

namespace mapnik { namespace json {

template <typename OutputIterator> struct feature_generator;

class geojson_generator 
{
    typedef std::back_insert_iterator<std::string> sink_type;
public:
    geojson_generator();
    ~geojson_generator();
    bool generate(std::string & geojson, mapnik::Feature const& f);
private:
    boost::scoped_ptr<feature_generator<sink_type> > grammar_;
};

}}


#endif //MAPNIK_GEOJSON_GENERATOR_HPP
