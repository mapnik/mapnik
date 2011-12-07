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

#ifndef OSMTAGTYPES_H
#define OSMTAGTYPES_H

// osmtagtypes.h
// for finding the types of particular tags

// mapnik
#include <mapnik/feature_layer_desc.hpp>

class osm_tag_types
{
public:
    void add_type(std::string tag, mapnik::eAttributeType type)
    {
        types[tag] = type;
    }

    mapnik::eAttributeType get_type(std::string tag)
    {
        std::map<std::string, mapnik::eAttributeType>::iterator i = types.find(tag);
        return (i == types.end()) ? mapnik::String : i->second;
    }

private:
    std::map<std::string, mapnik::eAttributeType> types;
};
  
#endif // OSMTAGTYPES_H
