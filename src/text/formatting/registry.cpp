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
#include <mapnik/text/formatting/registry.hpp>
#include <mapnik/text/formatting/text.hpp>
#include <mapnik/text/formatting/format.hpp>
#include <mapnik/text/formatting/layout.hpp>
#include <mapnik/xml_node.hpp>
#include <mapnik/config_error.hpp>

namespace mapnik
{
namespace formatting
{

registry::registry()
{
    register_name("<xmltext>", &text_node::from_xml);
    register_name("Format", &format_node::from_xml);
    // back compatibility shim - to be removed at mapnik 4
    register_name("ExpressionFormat", &format_node::from_xml);
    register_name("Layout", &layout_node::from_xml);
}

void registry::register_name(std::string const& name, from_xml_function_ptr ptr, bool overwrite)
{
    if (overwrite) {
        map_[name] = ptr;
    } else {
        map_.emplace(name, ptr);
    }
}

node_ptr registry::from_xml(xml_node const& xml, fontset_map const& fontsets)
{
    std::map<std::string, from_xml_function_ptr>::const_iterator itr = map_.find(xml.name());
    if (itr == map_.end())  throw config_error("Unknown element '" + xml.name() + "'", xml);
    xml.set_processed(true);
    return itr->second(xml, fontsets);
}
} //ns formatting
} //ns mapnik
