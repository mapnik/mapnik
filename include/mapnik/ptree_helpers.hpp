/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#ifndef MAPNIK_PTREE_HELPERS_HPP
#define MAPNIK_PTREE_HELPERS_HPP

// stl
#include <string>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/property_tree/ptree.hpp>
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

template<typename T>
void set_attr(boost::property_tree::ptree& pt, std::string const& name, T const& v)
{
    pt.put("<xmlattr>." + name, v);
}

} // end of namespace mapnik

#endif // MAPNIK_PTREE_HELPERS_HPP
