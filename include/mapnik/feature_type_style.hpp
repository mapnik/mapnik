/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

//$Id$

#ifndef FEATURE_TYPE_STYLE_HPP
#define FEATURE_TYPE_STYLE_HPP
// mapnik 
#include <mapnik/rule.hpp>
#include <mapnik/feature.hpp>
// stl
#include <vector>

namespace mapnik
{
    typedef std::vector<rule_type> rules;
    class feature_type_style
    {
    private:
        rules  rules_;
    public:
        feature_type_style() {}

        feature_type_style(feature_type_style const& rhs)
            : rules_(rhs.rules_) {}
	
        feature_type_style& operator=(feature_type_style const& rhs)
        {
            if (this == &rhs) return *this;
            rules_=rhs.rules_;
            return *this;
        }
	
        void add_rule(rule_type const& rule)
        {
            rules_.push_back(rule);
        } 
	
        rules const& get_rules() const
        {
            return rules_;
        }
        
        ~feature_type_style() {}
    };
}

#endif //FEATURE_TYPE_STYLE_HPP
