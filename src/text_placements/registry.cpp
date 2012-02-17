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
// mapnik
#include <mapnik/text_placements/registry.hpp>

namespace mapnik
{
namespace placements
{

registry::registry()
{

}

void registry::register_name(std::string name, from_xml_function_ptr ptr, bool overwrite)
{
    if (overwrite) {
        map_[name] = ptr;
    } else {
        map_.insert(make_pair(name, ptr));
    }
}

text_placements_ptr registry::from_xml(std::string name, const boost::property_tree::ptree &xml)
{
    std::map<std::string, from_xml_function_ptr>::const_iterator itr = map_.find(name);
    if (itr == map_.end())  throw config_error("Unknown placement-type '" + name + "'");
    return itr->second(xml);
}
} //ns formatting
} //ns mapnik
