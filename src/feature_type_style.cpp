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

#include <mapnik/feature_type_style.hpp>

namespace mapnik
{

static const char * filter_mode_strings[] = {
    "all",
    "first",
    ""
};

IMPLEMENT_ENUM( filter_mode_e, filter_mode_strings )


feature_type_style::feature_type_style()
    : filter_mode_(FILTER_ALL) {}

feature_type_style::feature_type_style(feature_type_style const& rhs, bool deep_copy)
    : filter_mode_(rhs.filter_mode_) 
{
    if (!deep_copy) {
        rules_ = rhs.rules_;
    } else {
        rules::const_iterator it  = rhs.rules_.begin(),
                        end = rhs.rules_.end();
        for(; it != end; ++it) {
            rules_.push_back(rule(*it, deep_copy));
        }
    }
}
    
feature_type_style& feature_type_style::operator=(feature_type_style const& rhs)
{
    if (this == &rhs) return *this;
    rules_=rhs.rules_;
    return *this;
}
    
void feature_type_style::add_rule(rule const& rule)
{
    rules_.push_back(rule);
} 
    
rules const& feature_type_style::get_rules() const
{
    return rules_;
}

rules &feature_type_style::get_rules_nonconst()
{
    return rules_;
}
    
void feature_type_style::set_filter_mode(filter_mode_e mode)
{
    filter_mode_ = mode;
}

filter_mode_e feature_type_style::get_filter_mode() const
{
    return filter_mode_;
}

}
