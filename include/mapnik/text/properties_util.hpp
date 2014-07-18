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

#ifndef MAPNIK_PROPERTIES_UTIL_HPP
#define MAPNIK_PROPERTIES_UTIL_HPP

#include <mapnik/symbolizer.hpp>
#include <mapnik/xml_node.hpp>
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/config_error.hpp>
#include <boost/optional.hpp>
#include <string>

namespace mapnik {

// text_layout_properties
template <typename T0, typename T1>
void set_property_from_xml(T1 & val, char const* name, xml_node  const& node)
{
    using target_type = T0;
    try
    {
        boost::optional<target_type> val_ = node.get_opt_attr<target_type>(name);
        if (val_) val = *val_;
    }
    catch (config_error const& ex)
    {
        boost::optional<expression_ptr> val_ = node.get_opt_attr<expression_ptr>(name);
        if (val_) val = *val_;
        else
        {
            ex.append_context(std::string("set_property_from_xml'"+ std::string(name) + "'"), node);
        }
    }
}

} // namespace mapnik

#endif // MAPNIK_PROPERTIES_UTIL_HPP
