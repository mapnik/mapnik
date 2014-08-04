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

#ifndef MAPNIK_ATTRIBUTE_HPP
#define MAPNIK_ATTRIBUTE_HPP

// mapnik
#include <mapnik/value_types.hpp>
#include <mapnik/value.hpp>

// stl
#include <string>
#include <unordered_map>

namespace mapnik {

struct attribute
{
    std::string name_;
    explicit attribute(std::string const& name)
        : name_(name) {}

    template <typename V ,typename F>
    V const& value(F const& f) const
    {
        return f.get(name_);
    }

    std::string const& name() const { return name_;}
};

struct geometry_type_attribute
{
    template <typename V, typename F>
    V value(F const& f) const
    {
        mapnik::value_integer type = 0;
        for (auto const& geom : f.paths())
        {
            if (type != 0 && geom.type() != type)
            {
                return value_integer(4); // Collection
            }
            type = geom.type();
        }
        return type;
    }
};

struct global_attribute
{
    std::string name;
    explicit global_attribute(std::string const& name_)
        : name(name_) {}

    template <typename V, typename C>
    V const& operator() (C const& ctx)
    {
        return ctx.get(name);
    }
};

using attributes = std::unordered_map<std::string, value>;

}

#endif // MAPNIK_ATTRIBUTE_HPP
