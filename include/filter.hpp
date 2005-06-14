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

#ifndef FILTER_HPP
#define FILTER_HPP

#include "filter_visitor.hpp"
#include "feature.hpp"
namespace mapnik
{
    typedef ref_ptr<filter<Feature> > filter_ptr;

    template <typename FeatureT> class filter_visitor;
    template <typename FeatureT>
    struct filter
    {
	virtual bool pass(const FeatureT& feature) const=0; 
	virtual filter<FeatureT>* clone() const=0;
	virtual void accept(filter_visitor<FeatureT>& v) = 0;
        virtual std::string to_string() const=0;
	virtual ~filter() {}
    };

    
    template <typename FeatureT>
    struct null_filter : public filter<FeatureT>
    {

	bool pass (const FeatureT&) const
	{
	    return true;
	}
	
	filter<FeatureT>* clone() const
	{
	    return new null_filter<FeatureT>;
	}
	std::string to_string() const
	{
	    return "true";
	}  
        void accept(filter_visitor<FeatureT>&) {}
	virtual ~null_filter() {}
    };
        
}

#endif //FILTER_HPP
