/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id$

#ifndef FEATURE_TYPE_STYLE
#define FEATURE_TYPE_STYLE

#include "rule.hh"
#include "feature.hh"
#include "ptr.hh"
#include <vector>

namespace mapnik
{
    
    //typedef feature<geometry_ptr,raster_ptr> Feature;
    typedef ref_ptr<rule<Feature,filter> > rule_ptr;

    class feature_type_style
    {
    private:
	std::vector<rule_ptr>  rules_;
    public:
	feature_type_style() {}
	feature_type_style(const feature_type_style& rhs)
	    : rules_(rhs.rules_) {}
	feature_type_style& operator=(const feature_type_style& rhs)
	{
	    if (this == &rhs) return *this;
	    rules_=rhs.rules_;
	    return *this;
	}
	void add_rule(const rule_ptr& rule)
	{
	    rules_.push_back(rule);
	} 
	const std::vector<rule_ptr>& rules() const
	{
	    return rules_;
	}
	
	~feature_type_style() {}
    };
}

#endif //FEATURE_TYPE_STYLE
