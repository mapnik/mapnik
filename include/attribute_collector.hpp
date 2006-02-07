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

#ifndef ATTRIBUTE_COLLECTOR_HPP
#define ATTRIBUTE_COLLECTOR_HPP

#include "filter.hpp"
#include "expression.hpp"
#include "feature_layer_desc.hpp"

#include <set>

namespace mapnik
{
    template <typename FeatureT>
    class attribute_collector : public filter_visitor<FeatureT>
    {
    public:
	attribute_collector(std::set<std::string>& names)
	    : names_(names) {}
	
	void visit(filter<FeatureT>& /*filter*/) 
	{ 
	    //not interested
	}
	
	void visit(expression<FeatureT>& exp)
	{
	    property<FeatureT>* pf;
	    if ((pf = dynamic_cast<property<FeatureT>*>(&exp)))
	    {
		names_.insert(pf->name());
	    }
	}	
	virtual ~attribute_collector() {}
    private:
	// no copying 
	attribute_collector(attribute_collector const&);
	attribute_collector& operator=(attribute_collector const&);
    private:
	std::set<std::string>& names_;
    };
}

#endif //ATTRIBUTE_COLLECTOR_HPP
