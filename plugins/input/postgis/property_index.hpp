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

#ifndef PROPERTY_VALIDATOR
#define PROPERTY_VALIDATOR

#include "filter.hh"
#include "expression.hh"
#include <set>

namespace mapnik
{
    template <typename FeatureT>
    class property_validator : public filter_visitor<FeatureT>
    {
    public:
	property_validator(layer_descriptor& desc)
	    desc_(desc) {}
	
	void visit(filter<FeatureT>& /*filter*/) 
	{ 
	    //not interested
	}
	void visit(expression<FeatureT>& exp)
	{
	    property<FeatureT>* pf;
	    if ((pf = dynamic_cast<property<FeatureT>*>(&exp)))
	    {
		vector<attribute_descriptor> const& attr_desc = desc_.get_descriptors();
		for (size_t idx=0; idx < attr_desc.size();++idx)
		{
		    if (attr_desc[idx] == pf->name())
		    {
			pf->set_index(idx);
			break;
		    }
		}	
	    }
	}
	std::set<std::string> const& property_names() const
	{
	    return names_;
	}
	
	virtual ~property_validator() {}
    private:
	property_validator(property_validator const&);
	property_validator& operator=(property_validator const&);
    private:
	layer_descriptor& desc_;
    };
}

#endif //PROPERTY_HPP
