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

#include "filter_visitor.hh"
#include <set>
#include "comparison.hh"

namespace mapnik
{
    template <typename Feature>
    class attribute_collector : public filter_visitor<Feature>
    {
    private:
	std::set<std::string> names_;
    public:
	attribute_collector() {}
	void visit(filter<Feature>& filter) 
	{   
	    property_filter<Feature>* pf_;
	    if((pf_=dynamic_cast<property_filter<Feature>*>(&filter)))
	    {
		names_.insert(pf_->name_);
	    }
	}
	const std::set<std::string>& property_names() const
	{
	    return names_;
	}
	
	virtual ~attribute_collector() {}
    private:
	attribute_collector(const attribute_collector&);
	attribute_collector& operator=(const attribute_collector&);
    };
}

#endif //ATTRIBUTE_COLLECTOR_HH
