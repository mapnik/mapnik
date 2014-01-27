/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_WKT_FACTORY_HPP
#define MAPNIK_WKT_FACTORY_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/wkt/wkt_grammar.hpp>
#include <mapnik/noncopyable.hpp>

// boost
#include <boost/ptr_container/ptr_vector.hpp>

// stl
#include <string>

namespace mapnik {

MAPNIK_DECL bool from_wkt(std::string const& wkt, boost::ptr_vector<geometry_type> & paths);

class MAPNIK_DECL wkt_parser : mapnik::noncopyable
{
    typedef std::string::const_iterator iterator_type;
public:
    wkt_parser();
    bool parse(std::string const& wkt, boost::ptr_vector<geometry_type> & paths);
private:
    const std::unique_ptr<mapnik::wkt::wkt_collection_grammar<iterator_type> > grammar_;
};

}


#endif // MAPNIK_WKT_FACTORY_HPP
