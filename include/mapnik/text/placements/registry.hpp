/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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
#include <mapnik/util/singleton.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/util/noncopyable.hpp>

// stl
#include <string>
#include <map>

namespace mapnik
{
namespace placements
{

using from_xml_function_ptr = text_placements_ptr (*) (xml_node const&, fontset_map const&, bool) ;

class registry : public singleton<registry, CreateStatic>,
                 private util::noncopyable
{
public:
    registry();
    ~registry() {}
    void register_name(std::string name, from_xml_function_ptr ptr, bool overwrite=false);
    text_placements_ptr from_xml(std::string name,
                                 xml_node const& xml,
                                 fontset_map const & fontsets,
                                 bool is_shield);
private:
    std::map<std::string, from_xml_function_ptr> map_;
};

} //ns placements
} //ns mapnik
#endif // PLACEMENTS_REGISTRY_HPP
