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

#ifndef QUERY_HPP
#define QUERY_HPP

#include <set>
#include <limits>
#include "filter.hpp"
#include "envelope.hpp"
#include "feature.hpp"

namespace mapnik
{
    class query 
    {
    private:
	Envelope<double> bbox_;
	filter<Feature>* filter_;
	std::set<std::string> names_;
    public:
	query() 
	    : bbox_(std::numeric_limits<double>::min(),
		    std::numeric_limits<double>::min(),
		    std::numeric_limits<double>::max(),
		    std::numeric_limits<double>::max()),
	      filter_(new null_filter<Feature>)
	{}
	
	query(const Envelope<double>& bbox)
	    : bbox_(bbox),
	      filter_(new null_filter<Feature>)
	{}
	
	query(const Envelope<double>& bbox,const filter<Feature>& f)
	    : bbox_(bbox),
	      filter_(f.clone())
	{}
	
	query(const query& other)
	    : bbox_(other.bbox_),
	      filter_(other.filter_->clone())
	{}
	
	query& operator=(const query& other)
	{
	    filter<Feature>* tmp=other.filter_->clone();
	    delete filter_;
	    filter_=tmp;
	    bbox_=other.bbox_;
	    names_=other.names_;
	    return *this;
	}
	
	const filter<Feature>* get_filter() const
	{
	    return  filter_;
	}
	
	const Envelope<double>& get_bbox() const
	{
	    return bbox_;
	}

	void set_filter(const filter<Feature>& f)
	{
	    filter<Feature>* tmp=f.clone();
	    delete filter_;
	    filter_=tmp;
	}
        
	void add_property_name(const std::string& name)
	{
	    names_.insert(name);
	} 
	
	const std::set<std::string>& property_names() const
	{
	    return names_;
	}
	
	~query() 
	{
	    delete filter_;
	}
    };
}


#endif //QUERY_HPP
