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
#ifndef PLACEMENTS_REGISTRY_HPP
#define PLACEMENTS_REGISTRY_HPP

// mapnik
#include <mapnik/utils.hpp>
#include <mapnik/text_placements/base.hpp>

// boost
#include <boost/utility.hpp>

// stl
#include <string>
#include <map>

namespace mapnik
{
namespace placements
{

typedef text_placements_ptr (*from_xml_function_ptr)(
    xml_node const& xml, fontset_map const & fontsets);

class registry : public singleton<registry, CreateStatic>,
                 private boost::noncopyable
{
public:
    registry();
    ~registry() {}
    void register_name(std::string name, from_xml_function_ptr ptr, bool overwrite=false);
    text_placements_ptr from_xml(std::string name,
                                 xml_node const& xml,
                                 fontset_map const & fontsets);
private:
    std::map<std::string, from_xml_function_ptr> map_;
};

} //ns placements
} //ns mapnik
#endif // PLACEMENTS_REGISTRY_HPP
