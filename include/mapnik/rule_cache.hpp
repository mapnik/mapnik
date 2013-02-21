/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

#ifndef MAPNIK_RULE_CACHE_HPP
#define MAPNIK_RULE_CACHE_HPP

// mapnik
#include <mapnik/rule.hpp>
#include <mapnik/feature_type_style.hpp>

// boost
#include <boost/foreach.hpp>

// stl
#include <vector>

namespace mapnik
{

class rule_cache
{
public:
    typedef std::vector<rule const*> rule_ptrs;
    rule_cache()
     : if_rules_(),
       else_rules_(),
       also_rules_() {}

    void add_rule(rule const& r)
    {
        if (r.has_else_filter())
        {
            else_rules_.push_back(&r);
        }
        else if (r.has_also_filter())
        {
            also_rules_.push_back(&r);
        }
        else
        {
            if_rules_.push_back(&r);
        }
    }

    rule_ptrs const& get_if_rules() const
    {
        return if_rules_;
    }
    
    rule_ptrs const& get_else_rules() const
    {
        return else_rules_;
    }
    
    rule_ptrs const& get_also_rules() const
    {
        return also_rules_;
    }

private:
    rule_ptrs if_rules_;
    rule_ptrs else_rules_;
    rule_ptrs also_rules_;
};

}

#endif // MAPNIK_RULE_CACHE_HPP
