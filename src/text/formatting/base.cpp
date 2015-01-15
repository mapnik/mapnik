/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#include <mapnik/debug.hpp>
#include <mapnik/text/formatting/base.hpp>
#include <mapnik/text/formatting/list.hpp>
#include <mapnik/text/formatting/registry.hpp>
#include <mapnik/xml_node.hpp>

namespace mapnik { namespace formatting {

node_ptr node::from_xml(xml_node const& xml, fontset_map const& fontsets)
{
    auto list = std::make_shared<list_node>();
    for (auto const& node : xml)
    {
        if (node.name() == "Placement")
            continue;
        node_ptr n = registry::instance().from_xml(node,fontsets);
        if (n) list->push_back(n);
    }
    if (list->get_children().size() == 1)
    {
        return list->get_children()[0];
    }
    else if (list->get_children().size() > 1)
    {
        return list;
    }
    else
    {
        return nullptr;
    }
}

} //ns formatting
} //ns mapnik
