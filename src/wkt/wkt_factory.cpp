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

// mapnik
#include <mapnik/wkt/wkt_factory.hpp>
#include <mapnik/wkt/wkt_grammar.hpp>

// stl
#include <string>
#include <sstream>
#include <stdexcept>

namespace mapnik
{

wkt_parser::wkt_parser()
    : grammar_(new mapnik::wkt::wkt_collection_grammar<iterator_type>)
{}

bool wkt_parser::parse(std::string const& wkt, boost::ptr_vector<geometry_type> & paths)
{
    using namespace boost::spirit;
    ascii::space_type space;
    iterator_type first = wkt.begin();
    iterator_type last =  wkt.end();
    return qi::phrase_parse(first, last, *grammar_, space, paths);
}

bool from_wkt(std::string const& wkt, boost::ptr_vector<geometry_type> & paths)
{
    wkt_parser parser;
    return parser.parse(wkt,paths);
}

}

