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

#ifndef FILTER_TO_STRING_HPP
#define FILTER_TO_STRING_HPP

#include "filter.hpp"
#include "expression.hpp"
#include <set>

namespace mapnik
{
    template <typename FeatureT>
    class filter_to_string : public filter_visitor<FeatureT>
    {
    private:
	std::string text_;
    public:
	filter_to_string() {}
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
	std::string const& text() const
	{
	    return text_;
	}
	
	virtual ~filter_to_string() {}
    private:
	filter_to_string(filter_to_string const&);
	filter_to_string& operator=(filter_to_string const&);
    };
}

#endif //FILTER_TO_STRING
