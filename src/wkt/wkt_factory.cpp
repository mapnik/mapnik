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

#include "wkt_factory.hpp"
#include "wkt_grammar.hpp"

#include <string>

namespace mapnik
{

std::pair<bool,geometry_type*> from_wkt(std::string const& wkt)
{
    using namespace boost::spirit;
    typedef std::string::const_iterator iterator_type;
    iterator_type first = wkt.begin();
    iterator_type last =  wkt.end();
    geometry_type * path = 0;
    mapnik::wkt::wkt_grammar<iterator_type> grammar;
    bool result =  qi::phrase_parse(first, last, grammar, ascii::space,path);
    return std::make_pair(result, path); 
}

}

