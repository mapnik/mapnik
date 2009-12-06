/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

#ifndef LIBXML2_LOADER_INCLUDED
#define LIBXML2_LOADER_INCLUDED

#include <boost/property_tree/ptree.hpp>

#include <string>

namespace mapnik 
{
    void read_xml2( std::string const & filename, boost::property_tree::ptree & pt);
    void read_xml2_string( std::string const & str, boost::property_tree::ptree & pt, std::string const & base_path="");
}

#endif // LIBXML2_LOADER_INCLUDED
