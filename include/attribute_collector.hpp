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
#include "rule.hpp"
#include <set>
#include <iostream>

namespace mapnik
{
    
    struct symbolizer_attributes : public boost::static_visitor<>
    {
	symbolizer_attributes(std::set<std::string>& names)
	    : names_(names) {}
	
	template <typename T>
	void operator () (T const&) const {}
	void operator () (text_symbolizer const& sym)
	{
	    names_.insert(sym.get_name());
	}
    private:
	std::set<std::string>& names_;
    };

    template <typename FeatureT>
    class attribute_collector : public filter_visitor<FeatureT>
    {
    private:
	std::set<std::string>& names_;
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
	void visit(rule_type const& r)
	{	    
	    const symbolizers& symbols = r.get_symbolizers();
	    symbolizers::const_iterator symIter=symbols.begin();
	    symbolizer_attributes attr(names_);
	    while (symIter != symbols.end())
	    {
		boost::apply_visitor(attr,*symIter++);
	    }
	    filter_ptr const& filter = r.get_filter();
	    filter->accept(*this);
	}

	virtual ~attribute_collector() {}
    private:
	
	// no copying 
	attribute_collector(attribute_collector const&);
	attribute_collector& operator=(attribute_collector const&);
    };   
}

#endif //ATTRIBUTE_COLLECTOR_HPP
