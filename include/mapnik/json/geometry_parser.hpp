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

#ifndef MAPNIK_JSON_GEOMETRY_PARSER_HPP
#define MAPNIK_JSON_GEOMETRY_PARSER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/geometry.hpp>

// boost
#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>
// stl
//#include <vector>

namespace mapnik { namespace json {

template <typename Iterator> struct geometry_grammar;

MAPNIK_DECL bool from_geojson(std::string const& json, boost::ptr_vector<geometry_type> & paths);

template <typename Iterator>
class MAPNIK_DECL geometry_parser : private boost::noncopyable
{
    typedef Iterator iterator_type;
public:
    geometry_parser();
    ~geometry_parser();
    bool parse(iterator_type first, iterator_type last, boost::ptr_vector<mapnik::geometry_type>&);
private:
    boost::scoped_ptr<geometry_grammar<iterator_type> > grammar_;
};

}}

#endif //MAPNIK_FEATURE_COLLECTION_PARSER_HPP
