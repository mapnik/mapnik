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

#ifndef PROPERTY_INDEX_HPP
#define PROPERTY_INDEX_HPP

#include "filter.hpp"
#include "expression.hpp"

#include <set>

namespace mapnik
{
    template <typename FeatureT>
    class property_index : public filter_visitor<FeatureT>
    {
    public:
	property_index(std::set<std::string> const& names)
	    : names_(names)  {}
	
	void visit(filter<FeatureT>& /*filter*/) 
	{ 
	    //not interested
	}
	void visit(expression<FeatureT>& exp)
	{
	    property<FeatureT>* pf;
	    if ((pf = dynamic_cast<property<FeatureT>*>(&exp)))
	    {
		std::set<std::string>::iterator pos;
		pos = names_.find(pf->name());
		if (pos != names_.end())
		{
		    size_t idx = std::distance(names_.begin(),pos);
		    pf->set_index(idx);
		}	
	    }
	}	
	virtual ~property_index() {}
    private:
	// no copying 
	property_index(property_index const&);
	property_index& operator=(property_index const&);

    private:
	std::set<std::string> const& names_;

    };
}

#endif //PROPERTY_INDEX_HPP
