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

#ifndef ATTRIBUTE_COLLECTOR
#define ATTROBUTE_COLLECTOR

#include "filter.hh"
#include "expression.hh"
#include <set>

namespace mapnik
{
    template <typename FeatureT>
    class attribute_collector : public filter_visitor<FeatureT>
    {
    private:
	std::set<std::string> names_;
    public:
	attribute_collector() {}
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
	std::set<std::string> const& property_names() const
	{
	    return names_;
	}
	
	virtual ~attribute_collector() {}
    private:
	attribute_collector(attribute_collector const&);
	attribute_collector& operator=(attribute_collector const&);
    };
}

#endif //ATTRIBUTE_COLLECTOR_HH
